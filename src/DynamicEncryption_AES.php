<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: AES Dynamic Encryption
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\MediaServices\Templates\TokenType;

require_once 'Config.inc';
require_once 'Common.inc';

$mezzanineFile = __DIR__.'/../media/BigBuckBunny.mp4';
$tokenType = TokenType::JWT;
$tokenRestriction = true;

echo "Azure Media Services PHP Sample - Dynamic Encryption AES \r\n";

echo "***** 1. Azure メディアサービス 接続 *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. ファイルアップロード *****\r\n";
$sourceAsset = uploadFileAndCreateAsset($restProxy, $mezzanineFile);

echo "***** 3. トランスコード *****\r\n";
$encodedAsset = encodeToAdaptiveBitrateMP4Set($restProxy, $sourceAsset);

echo "***** 4. コンテンツキーの作成 *****\r\n";
$contentKey = createEnvelopeTypeContentKey($restProxy, $encodedAsset);

echo "***** 5. コンテンツキーのAuthorizationポリシーの作成 *****\r\n";
$tokenTemplateString = null;
if ($tokenRestriction) {
    $tokenTemplateString = addTokenRestrictedAuthorizationPolicy_Envelope($restProxy, $contentKey, $tokenType);
} else {
    addOpenAuthorizationPolicy_Envelope($restProxy, $contentKey);
}

echo "***** 6. アセットデリバリーポリシーの作成 *****\r\n";
createAssetDeliveryPolicy_Envelope($restProxy, $encodedAsset, $contentKey);

echo "***** 7. 配信 *****\r\n";
publishEncodedAsset($restProxy, $encodedAsset);

echo "***** 8. テストトークン作成 *****\r\n";
if ($tokenRestriction) {
    generateTestToken($tokenTemplateString, $contentKey);
}

echo "***** 完了! *****\r\n";


?>

