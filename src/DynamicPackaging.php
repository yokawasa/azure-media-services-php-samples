<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Dynamic Packaging
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;

include_once 'Config.inc';
include_once 'Common.inc';

$mezzanineFile = __DIR__.'/../media/BigBuckBunny.mp4';

echo "Azure Media Services PHP Sample - Dynamic Packaging\r\n";

echo "***** 1. Azure メディアサービス 接続 *****\r\n";
$restProxy = ServicesBuilder::getInstance()->createMediaServicesService(
           new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. ファイルアップロード *****\r\n";
$sourceAsset = uploadFileAndCreateAsset($restProxy, $mezzanineFile);

echo "***** 3. トランスコード *****\r\n";
$encodedAsset = encodeToAdaptiveBitrateMP4Set($restProxy, $sourceAsset);

echo "***** 4. 配信 *****\r\n";
publishEncodedAsset($restProxy, $encodedAsset);

echo "***** 完了! *****\r\n";

?>

