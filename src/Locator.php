<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Locator manipulation 
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;
use WindowsAzure\MediaServices\Models\AccessPolicy;
use WindowsAzure\MediaServices\Models\Locator;

include_once 'Config.inc';

echo "**** Azure Media Services PHP Sample - Locator **** \r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Get Asset data by asset Id  *****\r\n";
$assetId  = "nb:cid:UUID:87dcbe30-8df8-42b5-949f-98f7c402b342";
$asset=$restProxy->getAsset($assetId);
var_dump($asset);

echo "***** 3. Get locator list of an asset *****\r\n";
$locators = $restProxy->getAssetLocators($assetId);
foreach ($locators as $loc){
    $locid = $loc -> getId();
    $ft = sprintf("Locator info: locatorId:%s assetId:%s assetPolicyId:%s path:%s\r\n",
                    $loc->getId(),
                    $loc->getAssetId(),
                    $loc->getAccessPolicyId(),
                    $loc->getPath());
    echo $ft;
}

echo "***** 4. Delete all locators of an asset *****\r\n";
foreach ($locators as $loc){
    $locid = $loc -> getId();
    $restProxy->deleteLocator($locid);
}

echo "***** 5. Create new locator *****\r\n";
// Create a 30-day read-only AccessPolicy
$access = new AccessPolicy('Streaming Access Policy');
$access->setDurationInMinutes(60 * 24 * 30);
$access->setPermissions(AccessPolicy::PERMISSIONS_READ);
$access = $restProxy->createAccessPolicy($access);
// Create a Locator using the AccessPolicy and Asset
$locator = new Locator($asset, $access, Locator::TYPE_ON_DEMAND_ORIGIN);
$locator->setName('Streaming Locator');
$locator = $restProxy->createLocator($locator);

echo "***** Done! *****\r\n";

?>

