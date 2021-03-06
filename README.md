# Azure Media Services PHP Samples

This repository contains PHP sample codes for Azure Media Services. Most of the sample codes here show how to use Azure PHP SDK for Azure Media Service. On the other hand, pure PHP sample codes for Azure Media Services are also included here for the one who doesn't want to depend on Azure PHP SDK.

## How to install Azure PHP SDK
The following procedure shows how to install [azure-sdk-for-php](https://github.com/Azure/azure-sdk-for-php) on your environent.

### create composer.json
    {
        "require": {        
            "microsoft/windowsazure": "^0.4"
        }  
    }

### download composer.phar
    wget http://getcomposer.org/composer.phar
    chmod +x composer.phar

### install Azure PHP SDK
    php composer.phar install

### install PHP mcrypt library
For the one who wants to run DynamicEncryption sample codes, PHP mcrypt library needs to be installed since it is required by Azure PHP SDK ContentKey model.

    apt-get install php5-mcrypt

## Sample Codes Configuration - Config.inc
Please edit Config.inc and add appropriate values for each parameters before run sample codes

    $config['accountname'] = '<Azure Media Services Account Name>';
    $config['accountkey'] = '<Azure Media Services Account Key>';
    $config['issuer'] = '<The secure token service that issues the token>';
    $config['audience'] = '<Audience (sometimes called scope) describes the intent of the token or the resource the token >';

## PHP SDK Sample Codes

You can find PHP sample codes under src directory. The followings are sample codes with short descriptions:
 * **Asset.php** - Azure PHP SDK Sample Code for Asset manipulation
 * **Locator.php** - Azure PHP SDK Sample Code for Locator manipulation
 * **EncodingReservedUnit.php** - Azure PHP SDK Sample Code for Encoding Reserved Unit manipulation
 * **MediaProcessor.php** - Azure PHP SDK Sample Code for Media Processor manipulation
 * **DynamicPackaging.php** - Azure PHP SDK Sample Code for Setting Dynamic Packaging and publishing
 * **DynamicEncryption_Create_AES.php** - Azure PHP SDK Sample Code for Setting AES Dynamic Encryption and publishing
 * **DynamicEncryption_Create_PR.php** - Azure PHP SDK Sample Code for Setting PlayReady Dynamic Encryption and publishing
 * **DynamicEncryption_Create_PR_WV.php** - Azure PHP SDK Sample Code for Setting PlayReady & Widevine Dynamic Encryption and publishing
 * **DynamicEncryption_Add_WV.php** - Azure PHP SDK Sample Code for Adding Widevine options to Current PlayReady DRM Configuration
 * **Common.inc** - Common libraries for Azure PHP SDK Sample Code for Azure Media Services

## Simple PHP REST API Sample Codes (NON PHP SDK Dependant)
Source code files that start from 'Simple' are NON PHP SDK dependant sample codes that you can find under src directory.
 * **SimplePHPAssetAPI.php** - Simple PHP Sample Code for Azure Media Services REST API Base Class
 * **SimplePHPAssetFilterAPI.php** - Simple PHP Sample Code for Azure Media Services: Asset Manipulation
 * **SimplePHPRestAPI.inc** - Simple PHP Sample Code for Azure Media Services: AssetFilter Manipulation

## TODOs
Adding more sample codes shortly. Your contribution is always welcomed!

