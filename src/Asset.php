<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Asset manipulation 
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;

include_once 'Config.inc';

echo "**** Azure Media Services PHP Sample - Asset **** \r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Get Asset data by asset Id  *****\r\n";
$Id ="nb:cid:UUID:3e243f13-5e2a-48f8-bebc-0a26fb96b391";
$asset=$restProxy->getAsset($Id);
//var_dump($asset);

echo "***** 3. Get Asset list *****\r\n";
$queryParams = ['$top' => 20, '$skip' => 40];
$assets=$restProxy->getAssetList($queryParams);
foreach( $assets as $asset ) {
    echo "name:" . $asset->getName() . "\r\n";
}

echo "***** 4. Update Asset *****\r\n";
$asset->setName("NewAssetName");
$restProxy->updateAsset($asset);

echo "***** 5. Delete Asset *****\r\n";
//$restProxy->deleteAsset($asset); // this also work
$restProxy->deleteAsset($Id);

echo "***** Done! *****\r\n";

?>

