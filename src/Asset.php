<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Asset manipulation 
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;

include_once 'Config.inc';

echo "**** Azure Media Services PHP Sample - Asset **** \r\n";

echo "***** 1. Azure メディアサービス 接続 *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. あるAssetIDのAsset情報を取得する *****\r\n";
$Id ="nb:cid:UUID:3e243f13-5e2a-48f8-bebc-0a26fb96b391";
$asset=$restProxy->getAsset($Id);
var_dump($asset);

echo "***** 3. Asset一覧の取得 *****\r\n";
$queryParams = ['$top' => 20, '$skip' => 40];
$assets=$restProxy->getAssetList($queryParams);
foreach( $assets as $asset ) {
    echo "name:" . $asset->getName() . "\n";
}


?>

