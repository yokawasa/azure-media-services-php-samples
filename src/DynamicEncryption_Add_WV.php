<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Dynamic Encryption 
//  - Adding WideVine to Current DRM Config by updating Asset Delivery Policy
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\MediaServices\Templates\TokenType;
use WindowsAzure\MediaServices\Models\ContentKeyDeliveryType;
use WindowsAzure\MediaServices\Models\ContentKeyTypes;

use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicy;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyOption;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyRestriction;
use WindowsAzure\MediaServices\Models\ContentKeyRestrictionType;

require_once 'Config.inc';
require_once 'Common.inc';

function is_empty ($input)
{
    return ( isset($input) && !empty($input) ) ? false : true;
}

function deleteLocators($restProxy, $assetId) {
    $locators = $restProxy->getAssetLocators($assetId);
    foreach ($locators as $loc){
        $locid = $loc -> getId();
        $restProxy->deleteLocator($locid);
    }
}

$assetId  = "nb:cid:UUID:bec16c65-fc9c-439a-98aa-b5f60ba69d79";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Get Updating Asset *****\r\n";
$asset=$restProxy->getAsset($assetId);

$locatorRemoved = false;
$needUpdateContentKey=false;

echo "***** 3. Get ContentKeys *****\r\n";
$contentKeys = $restProxy->getAssetContentKeys($assetId);

foreach ($contentKeys as $ck){
    $ckid = $ck -> getId();
    $cktype = $ck ->getContentKeyType(); 
    echo "Contentkey Id="  . $ckid . "\r\n";
    if ($cktype != ContentKeyTypes::COMMON_ENCRYPTION ) { 
        // skip if it's not CENC type
        continue;
    }

    echo "***** 4. Get Current AuthorizationPolicy or Create new AuthorizationPolicy If not created yet *****\r\n";
    //
    // Particulary targetting Showtime Asset doesn't have Authorization Policy that 
    // should be included in ContentKey. Thus, a new Authorization policy need to be 
    // created in order to add new WideVine ContentKey Authorization Policy Options.
    //
    $ckapolicy = null;
    $authpolicyid = $ck ->getAuthorizationPolicyId();
    echo "AuthorizationPolicyId Id="  . $authpolicyid . "\r\n";
    if (is_empty($authpolicyid)) { 
        // No Authorization Policy
        // Thus, create new ContentKeyAuthorizationPolicy that associated with the ContentKey
        echo "***** Create new ContentKey Authorization Policy *****\r\n";
        // Create ContentKeyAuthorizationPolicy
        $ckapolicy = new ContentKeyAuthorizationPolicy();
        $ckapolicy->setName('ContentKey Authorization Policy');
        $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);
        $needUpdateContentKey= true;
    } 
    else {
        // Get ContentKeyAuthorizationPolicy for the ContentKey
            //$ckapolicy = $restProxy->getContentKeyAuthorizationPolicy($authpolicyid);
           echo "***** Getting ContentKey Authorization Policy *****\r\n";
            try {
                $ckapolicy = $restProxy->getContentKeyAuthorizationPolicy($authpolicyid);
            } catch (Exception $e) {
                //echo "Caught exception: ", $e->getMessage(), "\n";
                echo "***** Caught exception, then creating new ContentKey Authorization Policy *****\r\n";
                $ckapolicy = new ContentKeyAuthorizationPolicy();
                $ckapolicy->setName('ContentKey Authorization Policy');
                $ckapolicy = $restProxy->createContentKeyAuthorizationPolicy($ckapolicy);
                $needUpdateContentKey= true;
            }
    }

    echo "***** 5. Update ContentKey by adding new WideVine ContentKey Authorization Policy Options *****\r\n";
    // Create ContentKeyAuthorizationPolicyRestriction (Open)
    $restriction = new ContentKeyAuthorizationPolicyRestriction();
    $restriction->setName('ContentKey Authorization Policy Restriction');
    $restriction->setKeyRestrictionType(ContentKeyRestrictionType::OPEN);
    // Configure Widevine license templates.
    $widevineLicenseTemplate = configureWidevineLicenseTemplate();
    // Create ContentKeyAuthorizationPolicyOption (Widevine)
    $widevineOption = new ContentKeyAuthorizationPolicyOption();
    $widevineOption->setName('Widevine Authorization Policy Option');
    $widevineOption->setKeyDeliveryType(ContentKeyDeliveryType::WIDEVINE);
    $widevineOption->setRestrictions(array($restriction));
    $widevineOption->setKeyDeliveryConfiguration($widevineLicenseTemplate);
    $widevineOption = $restProxy->createContentKeyAuthorizationPolicyOption($widevineOption);

    echo "***** linking Option To ContentKeyAuthorizationPolicy *****\r\n";
    // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
    $restProxy->linkOptionToContentKeyAuthorizationPolicy($widevineOption, $ckapolicy);
    if ( $needUpdateContentKey ) {
        echo "***** Updating ContentKey with new Authorization Policy *****\r\n";
        $ck->setAuthorizationPolicyId($ckapolicy->getId());
        $restProxy->updateContentKey($ck);
    }

    echo "***** 5. Delete Locators *****\r\n";
    if (!$locatorRemoved) {
        deleteLocators($restProxy, $assetId);
        $locatorRemoved = true;
    }

    echo "***** 6. Update Asset Delivery Policy by removing & adding new AssetDeliveryPolicy *****\r\n";
    //// [NOTE]
    //// If you try to add new asset delivery policy upon to existed one,
    //// you will get 409 error code ( 409 - Conflicts ).
    ////  Thus, remove first, then create new one
    //// https://msdn.microsoft.com/en-us/library/azure/dn168949.aspx

    // Remove current asset delivery policy
    $adpolicies = $restProxy->getAssetLinkedDeliveryPolicy($assetId);
    foreach ($adpolicies as $adpolicy){
        $restProxy->removeDeliveryPolicyFromAsset($asset, $adpolicy);
    }
    // Linking New Asset delivery policy
    $deliveryTypes=array();
    array_push($deliveryTypes, ContentKeyDeliveryType::PLAYREADY_LICENSE);
    array_push($deliveryTypes, ContentKeyDeliveryType::WIDEVINE);
    createAssetDeliveryPolicy_CENC($restProxy, $asset, $ck, $deliveryTypes);
}

echo "***** 7. Create new locator for publishing *****\r\n";
publishEncodedAsset($restProxy, $asset);

echo "***** Done! *****\r\n";

?>
