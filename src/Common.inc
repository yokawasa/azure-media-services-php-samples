<?php

//
// Common libraries for Azure PHP SDK Sample Code for Azure Media Services
// 
// Most of these common libraries here are originally from the following PHP SDK example codes. 
// https://github.com/Azure/azure-sdk-for-php/blob/master/examples/MediaServices/vodworkflow_aes.php
// https://github.com/Azure/azure-sdk-for-php/blob/master/examples/MediaServices/vodworkflow_drm_playready_widevine.php
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\Common\Internal\Utilities;
use WindowsAzure\MediaServices\Models\Asset;
use WindowsAzure\MediaServices\Models\AccessPolicy;
use WindowsAzure\MediaServices\Models\Locator;
use WindowsAzure\MediaServices\Models\Task;
use WindowsAzure\MediaServices\Models\Job;
use WindowsAzure\MediaServices\Models\TaskOptions;
use WindowsAzure\MediaServices\Models\ContentKey;
use WindowsAzure\MediaServices\Models\ProtectionKeyTypes;
use WindowsAzure\MediaServices\Models\ContentKeyTypes;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicy;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyOption;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyRestriction;
use WindowsAzure\MediaServices\Models\ContentKeyDeliveryType;
use WindowsAzure\MediaServices\Models\ContentKeyRestrictionType;
use WindowsAzure\MediaServices\Models\AssetDeliveryPolicy;
use WindowsAzure\MediaServices\Models\AssetDeliveryProtocol;
use WindowsAzure\MediaServices\Models\AssetDeliveryPolicyType;
use WindowsAzure\MediaServices\Models\AssetDeliveryPolicyConfigurationKey;
use WindowsAzure\MediaServices\Templates\PlayReadyLicenseResponseTemplate;
use WindowsAzure\MediaServices\Templates\PlayReadyLicenseTemplate;
use WindowsAzure\MediaServices\Templates\PlayReadyLicenseType;
use WindowsAzure\MediaServices\Templates\MediaServicesLicenseTemplateSerializer;
use WindowsAzure\MediaServices\Templates\WidevineMessage;
use WindowsAzure\MediaServices\Templates\AllowedTrackTypes;
use WindowsAzure\MediaServices\Templates\ContentKeySpecs;
use WindowsAzure\MediaServices\Templates\RequiredOutputProtection;
use WindowsAzure\MediaServices\Templates\Hdcp;
use WindowsAzure\MediaServices\Templates\TokenRestrictionTemplateSerializer;
use WindowsAzure\MediaServices\Templates\TokenRestrictionTemplate;
use WindowsAzure\MediaServices\Templates\SymmetricVerificationKey;
use WindowsAzure\MediaServices\Templates\TokenClaim;
use WindowsAzure\MediaServices\Templates\TokenType;
use WindowsAzure\MediaServices\Templates\WidevineMessageSerializer;

require_once 'Config.inc';

function uploadFileAndCreateAsset($restProxy, $mezzanineFileName)
{
    $fileName = basename($mezzanineFileName);

    // create an empty "Asset" by specifying the name
    $asset = new Asset(Asset::OPTIONS_NONE);
    //$asset->setName('Mezzanine '.$mezzanineFileName);
    $asset->setName('Mezzanine '. $fileName);
    $asset = $restProxy->createAsset($asset);
    $assetId = $asset->getId();

    echo 'Asset created: name='.$asset->getName().' id='.$assetId."\r\n";

    // create an Access Policy with Write permissions
    $accessPolicy = new AccessPolicy('UploadAccessPolicy');
    $accessPolicy->setDurationInMinutes(60.0);
    //$accessPolicy->setPermissions(AccessPolicy::PERMISSIONS_WRITE );
    $accessPolicy->setPermissions(AccessPolicy::PERMISSIONS_WRITE |AccessPolicy::PERMISSIONS_LIST );
    $accessPolicy = $restProxy->createAccessPolicy($accessPolicy);

    // create a SAS Locator for the Asset
    $sasLocator = new Locator($asset,  $accessPolicy, Locator::TYPE_SAS);
    $sasLocator = $restProxy->createLocator($sasLocator);

    // get the mezzanine file content
    $fileContent = file_get_contents($mezzanineFileName);

    echo "Uploading...\r\n";

    // use the 'uploadAssetFile' to perform a multi-part upload using the Block Blobs REST API storage operations
    $restProxy->uploadAssetFile($sasLocator, $fileName, $fileContent);

    // notify Media Services that the file upload operation is done to generate the asset file metadata
    $restProxy->createFileInfos($asset);

    echo 'File uploaded: size='.strlen($fileContent)."\r\n";

    // delete the SAS Locator (and Access Policy) for the Asset since we are done uploading files
    $restProxy->deleteLocator($sasLocator);
    $restProxy->deleteAccessPolicy($accessPolicy);

    return $asset;
}


/**
* Original function from 
* https://github.com/Azure/azure-sdk-for-php/blob/master/examples/MediaServices/vodworkflow_aes.php
*/
function encodeToAdaptiveBitrateMP4Set($restProxy, $asset)
{
    // Retrieve the latest 'Media Encoder Standard' processor version
    $mediaProcessor = $restProxy->getLatestMediaProcessor('Media Encoder Standard');

    echo "Using Media Processor: {$mediaProcessor->getName()} version {$mediaProcessor->getVersion()}\r\n";

    // Create the Job; this automatically schedules and runs it
    $outputAssetName = 'Encoded '.$asset->getName();
    $outputAssetCreationOption = Asset::OPTIONS_NONE;
    $taskBody = '<?xml version="1.0" encoding="utf-8"?><taskBody><inputAsset>JobInputAsset(0)</inputAsset><outputAsset assetCreationOptions="'.$outputAssetCreationOption.'" assetName="'.$outputAssetName.'">JobOutputAsset(0)</outputAsset></taskBody>';

    $task = new Task($taskBody, $mediaProcessor->getId(), TaskOptions::NONE);
    $task->setConfiguration('H264 Multiple Bitrate 720p');

    $job = new Job();
    $job->setName('Encoding Job');

    $job = $restProxy->createJob($job, array($asset), array($task));

    echo "Created Job with Id: {$job->getId()}\r\n";

    // Check to see if the Job has completed
    $result = $restProxy->getJobStatus($job);

    $jobStatusMap = array('Queued', 'Scheduled', 'Processing', 'Finished', 'Error', 'Canceled', 'Canceling');

    while ($result != Job::STATE_FINISHED && $result != Job::STATE_ERROR && $result != Job::STATE_CANCELED) {
        echo "Job status: {$jobStatusMap[$result]}\r\n";
        sleep(10);
        $result = $restProxy->getJobStatus($job);
    }

    if ($result != Job::STATE_FINISHED) {
        echo "The job has finished with a wrong status: {$jobStatusMap[$result]}\r\n";
        exit(-1);
    }

    echo "Job Finished!\r\n";

    // Get output asset
    $outputAssets = $restProxy->getJobOutputMediaAssets($job);
    $encodedAsset = $outputAssets[0];

    echo "Asset encoded: name={$encodedAsset->getName()} id={$encodedAsset->getId()}\r\n";

    return $encodedAsset;
}


function publishEncodedAsset($restProxy, $encodedAsset)
{
    // Get the .ISM AssetFile
    $files = $restProxy->getAssetAssetFileList($encodedAsset);
    $manifestFile = null;

    foreach ($files as $file) {
        if (endsWith(strtolower($file->getName()), '.ism')) {
            $manifestFile = $file;
        }
    }

    if ($manifestFile == null) {
        echo "Unable to found the manifest file\r\n";
        exit(-1);
    }

    // Create a 30-day read-only AccessPolicy
    $access = new AccessPolicy('Streaming Access Policy');
    $access->setDurationInMinutes(60 * 24 * 30);
    $access->setPermissions(AccessPolicy::PERMISSIONS_READ);
    $access = $restProxy->createAccessPolicy($access);

    // Create a Locator using the AccessPolicy and Asset
    $locator = new Locator($encodedAsset, $access, Locator::TYPE_ON_DEMAND_ORIGIN);
    $locator->setName('Streaming Locator');
    $locator = $restProxy->createLocator($locator);

    // Create a Smooth Streaming base URL
    $stremingUrl = $locator->getPath().$manifestFile->getName().'/manifest';

    echo "Streaming URL: {$stremingUrl}\r\n";
}

/* ---------------------------------------------------------------------- */
/*  Envelope Type Common Utils                                            */
/* ---------------------------------------------------------------------- */
function createEnvelopeTypeContentKey($restProxy, $encodedAsset)
{
    // Generate a new key
    $aesKey = Utilities::generateCryptoKey(16);

    // Get the protection key id for ContentKey
    $protectionKeyId = $restProxy->getProtectionKeyId(ContentKeyTypes::ENVELOPE_ENCRYPTION);
    $protectionKey = $restProxy->getProtectionKey($protectionKeyId);

    $contentKey = new ContentKey();
    $contentKey->setContentKey($aesKey, $protectionKey);
    $contentKey->setProtectionKeyId($protectionKeyId);
    $contentKey->setProtectionKeyType(ProtectionKeyTypes::X509_CERTIFICATE_THUMBPRINT);
    $contentKey->setContentKeyType(ContentKeyTypes::ENVELOPE_ENCRYPTION);

    // Create the ContentKey
    $contentKey = $restProxy->createContentKey($contentKey);

    echo "Content Key id={$contentKey->getId()}\r\n";

    // Associate the ContentKey with the Asset
    $restProxy->linkContentKeyToAsset($encodedAsset, $contentKey);

    return $contentKey;
}


function addOpenAuthorizationPolicy_Envelope($restProxy, $contentKey)
{
    // Create ContentKeyAuthorizationPolicyRestriction (Open)
    $restriction = new ContentKeyAuthorizationPolicyRestriction();
    $restriction->setName('ContentKey Authorization Policy Restriction');
    $restriction->setKeyRestrictionType(ContentKeyRestrictionType::OPEN);

    // Create ContentKeyAuthorizationPolicyOption (AES)
    $option = new ContentKeyAuthorizationPolicyOption();
    $option->setName('ContentKey Authorization Policy Option');
    $option->setKeyDeliveryType(ContentKeyDeliveryType::BASELINE_HTTP);
    $option->setRestrictions(array($restriction));
    $option = $restProxy->createContentKeyAuthorizationPolicyOption($option);

    // Create ContentKeyAuthorizationPolicy
    $ckapolicy = new ContentKeyAuthorizationPolicy();
    $ckapolicy->setName('ContentKey Authorization Policy');
    $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);

    // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
    $restProxy->linkOptionToContentKeyAuthorizationPolicy($option, $ckapolicy);

    // Associate the ContentKeyAuthorizationPolicy with the ContentKey
    $contentKey->setAuthorizationPolicyId($ckapolicy->getId());
    $restProxy->updateContentKey($contentKey);

    echo "Added Content Key Authorization Policy: name={$ckapolicy->getName()} id={$ckapolicy->getId()}\r\n";
}


function addTokenRestrictedAuthorizationPolicy_Envelope($restProxy, $contentKey, $tokenType)
{
    // Create ContentKeyAuthorizationPolicyRestriction (Token Restricted)
    $tokenRestriction = generateTokenRequirements($tokenType);
    $restriction = new ContentKeyAuthorizationPolicyRestriction();
    $restriction->setName('ContentKey Authorization Policy Restriction');
    $restriction->setKeyRestrictionType(ContentKeyRestrictionType::TOKEN_RESTRICTED);
    $restriction->setRequirements($tokenRestriction);

    // Create ContentKeyAuthorizationPolicyOption (AES)
    $option = new ContentKeyAuthorizationPolicyOption();
    $option->setName('ContentKey Authorization Policy Option');
    $option->setKeyDeliveryType(ContentKeyDeliveryType::BASELINE_HTTP);
    $option->setRestrictions(array($restriction));
    $option = $restProxy->createContentKeyAuthorizationPolicyOption($option);

    // Create ContentKeyAuthorizationPolicy
    $ckapolicy = new ContentKeyAuthorizationPolicy();
    $ckapolicy->setName('ContentKey Authorization Policy');
    $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);

    // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
    $restProxy->linkOptionToContentKeyAuthorizationPolicy($option, $ckapolicy);

    // Associate the ContentKeyAuthorizationPolicy with the ContentKey
    $contentKey->setAuthorizationPolicyId($ckapolicy->getId());
    $restProxy->updateContentKey($contentKey);

    echo "Added Content Key Authorization Policy: name={$ckapolicy->getName()} id={$ckapolicy->getId()}\r\n";

    return $tokenRestriction;
}


function createAssetDeliveryPolicy_Envelope($restProxy, $encodedAsset, $contentKey)
{
    // Get the acquisition URL
    $acquisitionUrl = $restProxy->getKeyDeliveryUrl($contentKey, ContentKeyDeliveryType::BASELINE_HTTP);

    // Generate the AssetDeliveryPolicy Configuration Key
    $randomKey = Utilities::generateCryptoKey(16);
    $configuration = [AssetDeliveryPolicyConfigurationKey::ENVELOPE_KEY_ACQUISITION_URL => $acquisitionUrl,
                      AssetDeliveryPolicyConfigurationKey::ENVELOPE_ENCRYPTION_IV_AS_BASE64 => base64_encode($randomKey), ];
    $confJson = AssetDeliveryPolicyConfigurationKey::stringifyAssetDeliveryPolicyConfiguartionKey($configuration);

    // Create the AssetDeliveryPolicy
    $adpolicy = new AssetDeliveryPolicy();
    $adpolicy->setName('Asset Delivery Policy');
    $adpolicy->setAssetDeliveryProtocol(AssetDeliveryProtocol::SMOOTH_STREAMING | AssetDeliveryProtocol::DASH | AssetDeliveryProtocol::HLS);
    $adpolicy->setAssetDeliveryPolicyType(AssetDeliveryPolicyType::DYNAMIC_ENVELOPE_ENCRYPTION);
    $adpolicy->setAssetDeliveryConfiguration($confJson);

    $adpolicy = $restProxy->createAssetDeliveryPolicy($adpolicy);

    // Link the AssetDeliveryPolicy to the Asset
    $restProxy->linkDeliveryPolicyToAsset($encodedAsset, $adpolicy->getId());

    echo "Added Asset Delivery Policy: name={$adpolicy->getName()} id={$adpolicy->getId()}\r\n";
}

/* ---------------------------------------------------------------------- */
/*  Envelope Type Common Utils                                            */
/* ---------------------------------------------------------------------- */

function createCommonTypeContentKey($restProxy, $encodedAsset)
{
    // Generate a new key
    $aesKey = Utilities::generateCryptoKey(16);

    // Get the protection key id for ContentKey
    $protectionKeyId = $restProxy->getProtectionKeyId(ContentKeyTypes::COMMON_ENCRYPTION);
    $protectionKey = $restProxy->getProtectionKey($protectionKeyId);

    $contentKey = new ContentKey();
    $contentKey->setContentKey($aesKey, $protectionKey);
    $contentKey->setProtectionKeyId($protectionKeyId);
    $contentKey->setProtectionKeyType(ProtectionKeyTypes::X509_CERTIFICATE_THUMBPRINT);
    $contentKey->setContentKeyType(ContentKeyTypes::COMMON_ENCRYPTION);

    // Create the ContentKey
    $contentKey = $restProxy->createContentKey($contentKey);

    echo "Content Key id={$contentKey->getId()}\r\n";

    // Associate the ContentKey with the Asset
    $restProxy->linkContentKeyToAsset($encodedAsset, $contentKey);

    return $contentKey;
}

/**
* deliveryTypes: array type: list of content delivery type
*/
function addOpenAuthorizationPolicy_CENC($restProxy, $contentKey, $deliveryTypes)
{
    // Create ContentKeyAuthorizationPolicyRestriction (Open)
    $restriction = new ContentKeyAuthorizationPolicyRestriction();
    $restriction->setName('ContentKey Authorization Policy Restriction');
    $restriction->setKeyRestrictionType(ContentKeyRestrictionType::OPEN);

    // Create ContentKeyAuthorizationPolicy
    $ckapolicy = new ContentKeyAuthorizationPolicy();
    $ckapolicy->setName('ContentKey Authorization Policy');
    $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);

    // PLAYREADY
    if (in_array(ContentKeyDeliveryType::PLAYREADY_LICENSE, $deliveryTypes ) )
    {
        // Configure PlayReady and Widevine license templates.
        $playReadyLicenseTemplate = configurePlayReadyLicenseTemplate();
        // Create ContentKeyAuthorizationPolicyOption (PlayReady)
        $playReadyOption = new ContentKeyAuthorizationPolicyOption();
        $playReadyOption->setName('PlayReady Authorization Policy Option');
        $playReadyOption->setKeyDeliveryType(ContentKeyDeliveryType::PLAYREADY_LICENSE);
        $playReadyOption->setRestrictions(array($restriction));
        $playReadyOption->setKeyDeliveryConfiguration($playReadyLicenseTemplate);
        $playReadyOption = $restProxy->createContentKeyAuthorizationPolicyOption($playReadyOption);
        // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
        $restProxy->linkOptionToContentKeyAuthorizationPolicy($playReadyOption, $ckapolicy);
    }

    // WIDEVINE
    if (in_array(ContentKeyDeliveryType::WIDEVINE, $deliveryTypes ) )
    {
        // Configure Widevine license templates.
        $widevineLicenseTemplate = configureWidevineLicenseTemplate();
        // Create ContentKeyAuthorizationPolicyOption (Widevine)
        $widevineOption = new ContentKeyAuthorizationPolicyOption();
        $widevineOption->setName('Widevine Authorization Policy Option');
        $widevineOption->setKeyDeliveryType(ContentKeyDeliveryType::WIDEVINE);
        $widevineOption->setRestrictions(array($restriction));
        $widevineOption->setKeyDeliveryConfiguration($widevineLicenseTemplate);
        $widevineOption = $restProxy->createContentKeyAuthorizationPolicyOption($widevineOption);
        // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
        $restProxy->linkOptionToContentKeyAuthorizationPolicy($widevineOption, $ckapolicy);
    }
    
    // Associate the ContentKeyAuthorizationPolicy with the ContentKey
    $contentKey->setAuthorizationPolicyId($ckapolicy->getId());
    $restProxy->updateContentKey($contentKey);

    echo "Added Content Key Authorization Policy: name={$ckapolicy->getName()} id={$ckapolicy->getId()}\r\n";
}

/**
* deliveryTypes: array type: list of content delivery type
*/
function addTokenRestrictedAuthorizationPolicy_CENC($restProxy, $contentKey, $tokenType, $deliveryTypes)
{
    // Create ContentKeyAuthorizationPolicyRestriction (Token Restricted)
    $tokenRestriction = generateTokenRequirements($tokenType);
    $restriction = new ContentKeyAuthorizationPolicyRestriction();
    $restriction->setName('ContentKey Authorization Policy Restriction');
    $restriction->setKeyRestrictionType(ContentKeyRestrictionType::TOKEN_RESTRICTED);
    $restriction->setRequirements($tokenRestriction);

    // Create ContentKeyAuthorizationPolicy
    $ckapolicy = new ContentKeyAuthorizationPolicy();
    $ckapolicy->setName('ContentKey Authorization Policy');
    $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);

    // PLAYREADY
    if (in_array(ContentKeyDeliveryType::PLAYREADY_LICENSE, $deliveryTypes ) )
    {
        // Configure PlayReady and Widevine license templates.
        $playReadyLicenseTemplate = configurePlayReadyLicenseTemplate();
        // Create ContentKeyAuthorizationPolicyOption (PlayReady)
        $playReadyOption = new ContentKeyAuthorizationPolicyOption();
        $playReadyOption->setName('PlayReady Authorization Policy Option');
        $playReadyOption->setKeyDeliveryType(ContentKeyDeliveryType::PLAYREADY_LICENSE);
        $playReadyOption->setRestrictions(array($restriction));
        $playReadyOption->setKeyDeliveryConfiguration($playReadyLicenseTemplate);
        $playReadyOption = $restProxy->createContentKeyAuthorizationPolicyOption($playReadyOption);
        // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
        $restProxy->linkOptionToContentKeyAuthorizationPolicy($playReadyOption, $ckapolicy);
    }

    // WIDEVINE
    if (in_array(ContentKeyDeliveryType::WIDEVINE, $deliveryTypes ) )
    {

        // Configure PlayReady and Widevine license templates.
        $widevineLicenseTemplate = configureWidevineLicenseTemplate();
        // Create ContentKeyAuthorizationPolicyOption (Widevine)
        $widevineOption = new ContentKeyAuthorizationPolicyOption();
        $widevineOption->setName('Widevine Authorization Policy Option');
        $widevineOption->setKeyDeliveryType(ContentKeyDeliveryType::WIDEVINE);
        $widevineOption->setRestrictions(array($restriction));
        $widevineOption->setKeyDeliveryConfiguration($widevineLicenseTemplate);
        $widevineOption = $restProxy->createContentKeyAuthorizationPolicyOption($widevineOption);
        // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
        $restProxy->linkOptionToContentKeyAuthorizationPolicy($widevineOption, $ckapolicy);
    }

    // Associate the ContentKeyAuthorizationPolicy with the ContentKey
    $contentKey->setAuthorizationPolicyId($ckapolicy->getId());
    $restProxy->updateContentKey($contentKey);

    echo "Added Content Key Authorization Policy: name={$ckapolicy->getName()} id={$ckapolicy->getId()}\r\n";

    return $tokenRestriction;
}


/**
* deliveryTypes: array type: list of content delivery type
*/
function createAssetDeliveryPolicy_CENC($restProxy, $encodedAsset, $contentKey, $deliveryTypes )
{
    $configuration = array();
    // PLAYREADY
    if (in_array(ContentKeyDeliveryType::PLAYREADY_LICENSE, $deliveryTypes ) )
    {
        // Get the acquisition URL
        $acquisitionUrl = $restProxy->getKeyDeliveryUrl($contentKey, ContentKeyDeliveryType::PLAYREADY_LICENSE);
        // Generate the AssetDeliveryPolicy Configuration Key
        $configuration[AssetDeliveryPolicyConfigurationKey::PLAYREADY_LICENSE_ACQUISITION_URL] = $acquisitionUrl;
    }
    // WIDEVINE
    if (in_array(ContentKeyDeliveryType::WIDEVINE, $deliveryTypes ) )
    {
        // Get the acquisition URL
        $widevineURl = $restProxy->getKeyDeliveryUrl($contentKey, ContentKeyDeliveryType::WIDEVINE);
        // Generate the AssetDeliveryPolicy Configuration Key
        $configuration[AssetDeliveryPolicyConfigurationKey::WIDEVINE_LICENSE_ACQUISITION_URL] = $widevineURl;
    }
    $confJson = AssetDeliveryPolicyConfigurationKey::stringifyAssetDeliveryPolicyConfiguartionKey($configuration);

    // Create the AssetDeliveryPolicy
    $adpolicy = new AssetDeliveryPolicy();
    $adpolicy->setName('Asset Delivery Policy');
    $adpolicy->setAssetDeliveryProtocol(AssetDeliveryProtocol::SMOOTH_STREAMING | AssetDeliveryProtocol::DASH);
    $adpolicy->setAssetDeliveryPolicyType(AssetDeliveryPolicyType::DYNAMIC_COMMON_ENCRYPTION);
    $adpolicy->setAssetDeliveryConfiguration($confJson);

    $adpolicy = $restProxy->createAssetDeliveryPolicy($adpolicy);

    // Link the AssetDeliveryPolicy to the Asset
    $restProxy->linkDeliveryPolicyToAsset($encodedAsset, $adpolicy->getId());

    echo "Added Asset Delivery Policy: name={$adpolicy->getName()} id={$adpolicy->getId()}\r\n";
}


function configurePlayReadyLicenseTemplate()
{
    // The following code configures PlayReady License Template using PHP classes
    // and returns the XML string.

    //The PlayReadyLicenseResponseTemplate class represents the template for the response sent back to the end user.
    //It contains a field for a custom data string between the license server and the application
    //(may be useful for custom app logic) as well as a list of one or more license templates.
    $responseTemplate = new PlayReadyLicenseResponseTemplate();

    // The PlayReadyLicenseTemplate class represents a license template for creating PlayReady licenses
    // to be returned to the end users.
    //It contains the data on the content key in the license and any rights or restrictions to be
    //enforced by the PlayReady DRM runtime when using the content key.
    $licenseTemplate = new PlayReadyLicenseTemplate();

    //Configure whether the license is persistent (saved in persistent storage on the client)
    //or non-persistent (only held in memory while the player is using the license).
    $licenseTemplate->setLicenseType(PlayReadyLicenseType::NON_PERSISTENT);

    // AllowTestDevices controls whether test devices can use the license or not.
    // If true, the MinimumSecurityLevel property of the license
    // is set to 150.  If false (the default), the MinimumSecurityLevel property of the license is set to 2000.
    $licenseTemplate->setAllowTestDevices(true);

    // You can also configure the Play Right in the PlayReady license by using the PlayReadyPlayRight class.
    // It grants the user the ability to playback the content subject to the zero or more restrictions
    // configured in the license and on the PlayRight itself (for playback specific policy).
    // Much of the policy on the PlayRight has to do with output restrictions
    // which control the types of outputs that the content can be played over and
    // any restrictions that must be put in place when using a given output.
    // For example, if the DigitalVideoOnlyContentRestriction is enabled,
    //then the DRM runtime will only allow the video to be displayed over digital outputs
    //(analog video outputs won't be allowed to pass the content).

    //IMPORTANT: These types of restrictions can be very powerful but can also affect the consumer experience.
    // If the output protections are configured too restrictive,
    // the content might be unplayable on some clients. For more information, see the PlayReady Compliance Rules document.

    // For example:
    //$licenseTemplate->getPlayRight()->setAgcAndColorStripeRestriction(new AgcAndColorStripeRestriction(1));

    $responseTemplate->setLicenseTemplates(array($licenseTemplate));

    return MediaServicesLicenseTemplateSerializer::serialize($responseTemplate);
}

function configureWidevineLicenseTemplate()
{
    $template = new WidevineMessage();
    $template->allowed_track_types = AllowedTrackTypes::SD_HD;
    $contentKeySpecs = new ContentKeySpecs();
    $contentKeySpecs->required_output_protection = new RequiredOutputProtection();
    $contentKeySpecs->required_output_protection->hdcp = Hdcp::HDCP_NONE;
    $contentKeySpecs->security_level = 1;
    $contentKeySpecs->track_type = 'SD';
    $template->content_key_specs = array($contentKeySpecs);
    $policyOverrides = new \stdClass();
    $policyOverrides->can_play = true;
    $policyOverrides->can_persist = true;
    $policyOverrides->can_renew = false;
    $template->policy_overrides = $policyOverrides;

    return WidevineMessageSerializer::serialize($template);
}


function generateTokenRequirements($tokenType)
{
    global $config;
    $template = new TokenRestrictionTemplate($tokenType);
    $template->setPrimaryVerificationKey(new SymmetricVerificationKey());
    $template->setAudience($config['audience']);
    $template->setIssuer($config['issuer']);
    $claims = array();
    $claims[] = new TokenClaim(TokenClaim::CONTENT_KEY_ID_CLAIM_TYPE);
    $template->setRequiredClaims($claims);

    return TokenRestrictionTemplateSerializer::serialize($template);
}

function generateTestToken($tokenTemplateString, $contentKey)
{
    $template = TokenRestrictionTemplateSerializer::deserialize($tokenTemplateString);
    $contentKeyUUID = substr($contentKey->getId(), strlen('nb:kid:UUID:'));
    $expiration = strtotime('+12 hour');
    $token = TokenRestrictionTemplateSerializer::generateTestToken($template, null, $contentKeyUUID, $expiration);

    echo "Token Type {$template->getTokenType()}\r\nBearer={$token}\r\n";
}

function endsWith($haystack, $needle)
{
    $length = strlen($needle);
    if ($length == 0) {
        return true;
    }

    return substr($haystack, -$length) === $needle;
}


?>
