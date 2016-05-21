<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: PlayReady + WideVine Dynamic Encryption
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
$tokenRestriction = true;

echo "Azure Media Services PHP Sample - Dynamic Encryption PlayReady & WideVine \r\n";

echo "***** 1. Azure メディアサービス 接続 *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. ファイルアップロード *****\r\n";
$sourceAsset = uploadFileAndCreateAsset($restProxy, $mezzanineFile);

echo "***** 3. トランスコード *****\r\n";
$encodedAsset = encodeToAdaptiveBitrateMP4Set($restProxy, $sourceAsset);

echo "***** 4. コンテンツキーの作成 *****\r\n";
$contentKey = createCommonTypeContentKey($restProxy, $encodedAsset);

$deliveryTypes=array();
$deliveryTypes.array_push($deliveryTypes, ContentKeyDeliveryType::PLAYREADY_LICENSE);
$deliveryTypes.array_push($deliveryTypes, ContentKeyDeliveryType::WIDEVINE);
echo "***** 5. コンテンツキーのAuthorizationポリシーの作成 *****\r\n";
$tokenTemplateString = null;
if ($tokenRestriction) {
    $tokenTemplateString = addTokenRestrictedAuthorizationPolicy_CENC($restProxy, $contentKey, $tokenType, $deliveryTypes);
} else {
    addOpenAuthorizationPolicy_CENC($restProxy, $contentKey, $deliveryTypes);
}

echo "***** 6. アセットデリバリーポリシーの作成 *****\r\n";
createAssetDeliveryPolicy_CENC($restProxy, $encodedAsset, $contentKey, $deliveryTypes);

echo "***** 7. 配信 *****\r\n";
publishEncodedAsset($restProxy, $encodedAsset);

echo "***** 8. テストトークン作成 *****\r\n";
if ($tokenRestriction) {
    generateTestToken($tokenTemplateString, $contentKey);
}

echo "***** 完了! *****\r\n";

?>

