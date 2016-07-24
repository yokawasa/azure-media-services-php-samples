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

use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicy;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyOption;
use WindowsAzure\MediaServices\Models\ContentKeyAuthorizationPolicyRestriction;
use WindowsAzure\MediaServices\Models\ContentKeyRestrictionType;

require_once 'Config.inc';
require_once 'Common.inc';


echo "Azure Media Services PHP Sample - Add WideVine to Current DRM Config by updating Asset Delivery Policy \r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

$assetId  = "nb:cid:UUID:bec16c65-fc9c-439a-98aa-b5f60ba69d79";

echo "***** 2. Get Updating Asset *****\r\n";
$asset=$restProxy->getAsset($assetId);
//var_dump($asset);

echo "***** 3. Delete Locators *****\r\n";
$locators = $restProxy->getAssetLocators($assetId);
foreach ($locators as $loc){
    $locid = $loc -> getId();
    $restProxy->deleteLocator($locid);
}

echo "***** 4. Get ContentKeys *****\r\n";
$contentKeys = $restProxy->getAssetContentKeys($assetId);
//var_dump($contentKeys);
foreach ($contentKeys as $ck){
    $ckid = $ck -> getId();
    //echo "Contentkey Id="  . $ckid . "\r\n";
    $authpolicyid = $ck ->getAuthorizationPolicyId();

    echo "***** 5. Update ContentKey by adding new WideVine ContentKey Authorization Policy Options *****\r\n";
    // Get ContentKeyAuthorizationPolicy for the ContentKey
    $ckapolicy = $restProxy->getContentKeyAuthorizationPolicy($authpolicyid);
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
    // Link the ContentKeyAuthorizationPolicyOption to the ContentKeyAuthorizationPolicy
    $restProxy->linkOptionToContentKeyAuthorizationPolicy($widevineOption, $ckapolicy);
    $ck->setAuthorizationPolicyId($ckapolicy->getId());
    // Updating ContentKey
    $restProxy->updateContentKey($ck);

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

