<?php

//
// Pure PHP Sample Code for Azure Media Services REST API Base Class
//
const TOKEN_STORAGE = '/tmp/AMS-TOKEN_STORAGE';
const AMS_OAUTH_URL = 'https://wamsprodglobal001acs.accesscontrol.windows.net/v2/OAuth2-13';
const AMS_API_BASE_URL = 'https://media.windows.net/API/';

/**
* Pure PHP REST API Base Class
*/
class RestAPI {
    protected $accountid;
    protected $accesskey;
    protected $baseURL;
    protected $strUrl;
    protected $strToken;
    protected $strBody;
    public function __construct($accountid, $accesskey){
        $this->accountid = $accountid;
        $this->accesskey= $accesskey;
        $this->getToken();
    }

    /**
    * Get Auth Token either from local file storage or from Token Auth API
    *
    * @return auth token string
    */
    public function getToken(){
        if (file_exists(TOKEN_STORAGE)){
            // Get Token from storage
            $data = file_get_contents(TOKEN_STORAGE);
            $tokenArr = json_decode($data);
            // Check if token expired
            if ((filemtime(TOKEN_STORAGE) + $tokenArr->expires_in) > time()) {
                // Token is NOT expired, thus get the token from local file storage
                $this->strToken = $tokenArr->access_token;
            } else {
                // Token is expired, thus remove local file cache and get it from API 
                unlink(TOKEN_STORAGE);
                $this->getToken();
            }
        } else {
            // Get new token from API
            $dataArr = array(
                'grant_type=client_credentials',
                'client_id='. $this->accountid,
                'client_secret='.urlencode($this->accesskey),
                'scope=urn%3aWindowsAzureMediaServices'
            );
            $postFields = implode('&', $dataArr);
            $headerArr = array(
                'Content-length:'.strlen($postFields),
                'Content-Type: application/x-www-form-urlencoded',
                'Connection: Keep-Alive',
                'Accept: application/json'
            );
 
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, AMS_OAUTH_URL);
            curl_setopt($ch, CURLOPT_POST, true);
            curl_setopt($ch, CURLOPT_POSTFIELDS, $postFields);
            curl_setopt($ch, CURLOPT_HTTPHEADER, $headerArr);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            $data = curl_exec($ch);
            curl_close($ch);
            $tokenArr = json_decode($data);
            if (isset($tokenArr->error)){
                print_r($tokenArr);
                die();
            }
            $this->strToken = $tokenArr->access_token;
            // Token save in storage
            file_put_contents(TOKEN_STORAGE, $data);
        }
        return $this->strToken;
    }
 
    public function requestGET(){
        $ch = curl_init();
        $headerArr = array(
            'x-ms-version:2.11',                // latest version as of 2016-01
            'Authorization: Bearer '.$this->strToken,
            'Content-Type: application/json',
            'Accept-Charset: UTF-8',
            'Accept: application/json'
        );
        curl_setopt($ch, CURLOPT_URL, $this->strUrl);
        curl_setopt($ch, CURLOPT_HTTPHEADER, $headerArr);
        curl_setopt($ch, CURLOPT_HEADER, true);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        $data = curl_exec($ch);
        $infoArr = curl_getinfo($ch);
        curl_close($ch);
 
        list($strHeader, $strBody) = explode("\r\n\r\n", $data, 2);
        if ($infoArr['http_code'] == 301){
            if(preg_match('`\s*Location:\s(.+)`i', $data, $tmpArr)){
                $tmpArr[1] = trim($tmpArr[1]);
                $this->baseURL = $tmpArr[1];
                $this->strUrl = $this->baseURL.$this->getVerb();
                $this->requestGET();
            }
        } else {
            $this->strBody = $strBody;
        }
    }

    public function requestPOST( $dataArr=array() ){
        $ch = curl_init();
        $headerArr = array(
            'x-ms-version:2.11',        // latest version as of 2016-01
            'Authorization: Bearer '.$this->strToken,
            'Content-Type: application/json',
            'Accept-Charset: UTF-8',
            'Accept: application/json'
        );
        curl_setopt($ch, CURLOPT_URL, $this->strUrl);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, $headerArr);
        curl_setopt($ch, CURLOPT_HEADER, true);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        if (!empty($dataArr)){
//var_dump(json_encode($dataArr));
            curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($dataArr));
        }
        $data = curl_exec($ch);
        $infoArr = curl_getinfo($ch);
        curl_close($ch);
 
        list($strHeader, $strBody) = explode("\r\n\r\n", $data, 2);
        if ($infoArr['http_code'] == 301){
            if(preg_match('`\s*Location:\s(.+)`i', $data, $tmpArr)){
                $tmpArr[1] = trim($tmpArr[1]);
                $this->baseURL = $tmpArr[1];
                $this->strUrl = $this->baseURL.$this->getVerb();
                $this->requestPOST($dataArr);
            }
        } else {
            $this->strBody = $strBody;
        }
    }
 
    private function getVerb(){
        $urlArr = explode('/', $this->strUrl);
        return array_pop($urlArr);
    }
}


?>
