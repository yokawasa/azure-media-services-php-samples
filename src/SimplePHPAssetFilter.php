<?php

//
// Simple PHP Sample Code for Azure Media Services: AssetFilter Manipulation
//

include_once 'Config.inc';
include_once 'SimplePHPRestAPI.inc';

/**
* Simple AssetFilter API Class that extends RestAPI
*/
class AssetFilterAPI extends RestAPI{
 
    public function __construct($accountid, $accesskey){
        parent::__construct($accountid, $accesskey);
    }

    public function createAssetFilter($filterArr) {
        $strAPIname = 'AssetFilters';
        $this->strUrl = AMS_API_BASE_URL.$strAPIname;
        $this->requestPOST($filterArr);
        return $this->strBody;
    }
}


/* ----------------------------------------------------------------*/
//                   TEST CODE                                     //
/* ----------------------------------------------------------------*/

$filterArr = array (
  'Name' => 'TestFilter1',
  'ParentAssetId' => 'nb:cid:UUID:6205403a-0d00-80c4-fd44-f1e58f7bad89',
  'PresentationTimeRange' => array (
    'StartTimestamp' => '0',
    'EndTimestamp' => '80000000',
    'LiveBackoffDuration' => '0',
    'Timescale' => '10000000',
  ),
  'Tracks' => array (
    0 => array (
      'PropertyConditions' => array (
        0 => array (
          'Property' => 'Type',
          'Value' => 'audio',
          'Operator' => 'Equal',
        ),
        1 => array (
          'Property' => 'Bitrate',
          'Value' => '503219-503219',
          'Operator' => 'Equal',
        ),
      ),
    ),
  ),
);

$afapi = new AssetFilterAPI($config['accountname'], $config['accountkey']);
$str = $afapi->createAssetFilter($filterArr);
print_r(json_decode($str));


?>
