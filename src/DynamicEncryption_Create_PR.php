<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: PlayReady Dynamic Encryption
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\MediaServices\Templates\TokenType;
use WindowsAzure\MediaServices\Models\ContentKeyDeliveryType;

require_once 'Config.inc';
require_once 'Common.inc';

$mezzanineFile = __DIR__.'/../media/BigBuckBunny.mp4';
$tokenType = TokenType::JWT;
//$tokenRestriction = true;
$tokenRestriction = false;

echo "Azure Media Services PHP Sample - Dynamic Encryption PlayReady \r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Upload File *****\r\n";
$sourceAsset = uploadFileAndCreateAsset($restProxy, $mezzanineFile);

echo "***** 3. Transcode *****\r\n";
$encodedAsset = encodeToAdaptiveBitrateMP4Set($restProxy, $sourceAsset);

echo "***** 4. Create ContentKey *****\r\n";
$contentKey = createCommonTypeContentKey($restProxy, $encodedAsset);

$deliveryTypes=array();
$deliveryTypes.array_push($deliveryTypes, ContentKeyDeliveryType::PLAYREADY_LICENSE);

echo "***** 5. Create Authorization Policy for the ContentKey *****\r\n";
$tokenTemplateString = null;
if ($tokenRestriction) {
    $tokenTemplateString = addTokenRestrictedAuthorizationPolicy_CENC($restProxy, $contentKey, $tokenType, $deliveryTypes);
} else {
    addOpenAuthorizationPolicy_CENC($restProxy, $contentKey, $deliveryTypes);
}

echo "***** 6. Create Asset Delivery Policy *****\r\n";
createAssetDeliveryPolicy_CENC($restProxy, $encodedAsset, $contentKey, $deliveryTypes);

echo "***** 7. Create Locator for publishing *****\r\n";
publishEncodedAsset($restProxy, $encodedAsset);

echo "***** 8. Generate Test Token *****\r\n";
if ($tokenRestriction) {
    generateTestToken($tokenTemplateString, $contentKey);
}

echo "***** Done! *****\r\n";


?>

