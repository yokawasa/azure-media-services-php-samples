<?php

//
// Azure PHP SDK Sample Code for Azure Media Services: Media Processor manipulation 
//

require_once __DIR__.'/../vendor/autoload.php';

use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Common\Internal\MediaServicesSettings;

include_once 'Config.inc';

echo "**** Azure Media Services PHP Sample - Media Processor **** \r\n";

echo "***** 1. Connect to Azure Media Services *****\r\n";
$restProxy = ServicesBuilder::getInstance()-> createMediaServicesService(
            new MediaServicesSettings($config['accountname'], $config['accountkey']));

echo "***** 2. Get All Media Processors  *****\r\n";
$processors=$restProxy->getMediaProcessors();
//var_dump($processors);
foreach( $processors as $processor ) {
    $ft = sprintf("Processor Name: %s (Version: %s)\r\n", 
                            $processor->getName(),
                            $processor->getVersion() );
    echo $ft;
}

echo "***** Done! *****\r\n";

?>

