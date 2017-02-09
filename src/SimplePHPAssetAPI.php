<?php

//
// Simple PHP Sample Code for Azure Media Services: Asset Manipulation
//

require_once 'Config.inc';
require_once 'SimplePHPRestAPI.inc';

/**
* Simple Asset API Class that extends RestAPI
*/
class AssetAPI extends RestAPI{
 
    public function __construct($accountid, $accesskey){
        parent::__construct($accountid, $accesskey);
    }

    // https://docs.microsoft.com/en-us/rest/api/media/operations/asset#create_an_asset
    public function createAsset($assetArr) {
        $strAPIname = 'Assets';
        $this->strUrl = AMS_API_BASE_URL.$strAPIname;
        $this->requestPost($assetArr);
        return $this->strBody;
    }

    // https://docs.microsoft.com/en-us/rest/api/media/operations/asset#list_an_asset 
    public function listAsset() {
        $strAPIname = 'Assets';
        $this->strUrl = AMS_API_BASE_URL.$strAPIname;
        $this->requestGET();
        return $this->strBody;
    }
}


/* ----------------------------------------------------------------*/
//                   TEST CODE                                     //
/* ----------------------------------------------------------------*/

$aapi = new AssetAPI($config['accountname'], $config['accountkey']);

$assetArr=  array (
  'Name' => 'TestAsset1',
);

echo "Creating Asset ....\n";
$str = $aapi->createAsset($assetArr);
print_r(json_decode($str));

echo "Listing Asset ....\n";
$str = $aapi->listAsset();
print_r(json_decode($str));

?>
