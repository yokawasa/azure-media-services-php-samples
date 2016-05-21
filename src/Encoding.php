<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Encoding Manipulation
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\MediaServices\Models\EncodingReservedUnitType;

include_once 'Config.inc';

$reservedUnits = 1;
$reservedUnitsType = EncodingReservedUnitType::S1;
$types = array('S1', 'S2', 'S3');

echo "Azure Media Services PHP Sample - Scale Encoding Units Sample\r\n";

echo "***** 1. Azure メディアサービス 接続 *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. 現在のエンコーディングユニット設定値を取得 *****\r\n";
$encodingUnits = $restProxy->getEncodingReservedUnit();
echo '現在のエンコーディングユニット値: '.$encodingUnits->getCurrentReservedUnits().' units ('.$types[$encodingUnits->getReservedUnitType()].")\r\n";

echo "***** 3. エンコーディングユニット設定値の更新 *****\r\n";
$encodingUnits->setCurrentReservedUnits($reservedUnits);
$encodingUnits->setReservedUnitType($reservedUnitsType);

echo "***** 4. エンコーディングリザーブドユニットの更新 *****\r\n";
$restProxy->updateEncodingReservedUnit($encodingUnits);

echo "***** 5. 現在のエンコーディング設定を読み込んで結果表示 *****\r\n";
$encodingUnits = $restProxy->getEncodingReservedUnit();
echo "\r\n更新後のエンコーディングユニット値: ".$encodingUnits->getCurrentReservedUnits().' units ('.$types[$encodingUnits->getReservedUnitType()].")\r\n";


?>
