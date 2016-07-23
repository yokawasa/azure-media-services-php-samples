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

echo "Azure Media Services PHP Sample - Encoding Reserved Units Operation Sample\r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Get Current Encoding Reserved Unit *****\r\n";
$encodingUnits = $restProxy->getEncodingReservedUnit();
$ft = sprintf("Current Encoding Reserved Unit: %s units (%s)\r\n",
                $encodingUnits->getCurrentReservedUnits(),
                $types[$encodingUnits->getReservedUnitType()]);
echo $ft;

echo "***** 3. Set encoding reserved unit Type *****\r\n";
$encodingUnits->setCurrentReservedUnits($reservedUnits);
$encodingUnits->setReservedUnitType($reservedUnitsType);

echo "***** 4. Update Encoding Reserved Unit *****\r\n";
$restProxy->updateEncodingReservedUnit($encodingUnits);

echo "***** 5. Get Updated Encoding Reserved Unit *****\r\n";
$encodingUnits = $restProxy->getEncodingReservedUnit();
$ft = sprintf("Updated Encoding Reserved Unit: %s units (%s)\r\n",
                $encodingUnits->getCurrentReservedUnits(),
                $types[$encodingUnits->getReservedUnitType()]);
echo $ft;

echo "***** Done! *****\r\n";

?>
