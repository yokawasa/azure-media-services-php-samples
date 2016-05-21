<?php

//
// Pure PHP Sample Code for Azure Media Services: Asset Manipulation
//

require_once 'Config.inc';
require_once 'PurePHPRestAPI.inc';

/**
* Asset API Class that extends RestAPIBase
*/
class AssetAPI extends RestAPI{
 
    public function __construct($accountid, $accesskey){
        parent::__construct($accountid, $accesskey);
    }
 
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
$str = $aapi->listAsset();
print_r(json_decode($str));

?>
