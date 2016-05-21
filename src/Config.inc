<?php

//
// Configuraion file
//

set_time_limit(600); //set timeout to 10 minutes, or you can set max_execution_time in php.ini

$config['accountname'] = '<Azure Media Services Account Name>';
$config['accountkey'] = '<Azure Media Services Account Key>';
$config['issuer'] = '<The secure token service that issues the token>';
$config['audience'] = '<Audience (sometimes called scope) describes the intent of the token or the resource the token >';

//$config['accountname'] = 'yoichitest';
//$config['accountkey'] = '7wgx+hMgxxt2MLMpjxPvS1o267biw/Sg5PiTa/yVH6A=';
//$config['issuer'] = 'https://contoso.accesscontrol.windows.net';
//$config['audience'] = 'urn:contoso';

?>
