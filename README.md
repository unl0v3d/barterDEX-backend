# barterDEX Client "marketmaker"

OS | Build Status 
-------------|------
Unix (Ubuntu 14.04) | [![Build Status](https://jenkinsmaster.sprnt.pw/buildStatus/icon?job=iguana-unix-jl777-release-v0.1)](https://jenkinsmaster.sprnt.pw/job/iguana-unix-jl777-release-v0.1)
Chrome | [![Build Status](https://jenkinsmaster.sprnt.pw/buildStatus/icon?job=iguana-pnacl-jl777-release-v0.1)](https://jenkinsmaster.sprnt.pw/job/iguana-pnacl-jl777-release-v0.1/)
Android | [![Build Status](https://jenkinsmaster.sprnt.pw/buildStatus/icon?job=iguana-android-jl777-release-v0.1)](https://jenkinsmaster.sprnt.pw/job/iguana-android-jl777-release-v0.1/)
iOS | [![Build Status](https://jenkinsmaster.sprnt.pw/buildStatus/icon?job=iguana-ios-jl777-release-v0.1)](https://jenkinsmaster.sprnt.pw/job/iguana-ios-jl777-release-v0.1/)
Windows 32 Bit | [![Build Status](https://jenkinsmaster.sprnt.pw/job/iguana-win32-jl777-release-v0.1/badge/icon)](https://jenkinsmaster.sprnt.pw/job/iguana-win32-jl777-release-v0.1/)
Windows 64 Bit | [![Build Status](https://jenkinsmaster.sprnt.pw/job/iguana-win64-jl777-release-v0.1/badge/icon)](https://jenkinsmaster.sprnt.pw/job/iguana-win64-jl777-release-v0.1/)
docs.supernet.org | [![Build Status](https://jenkinsmaster.sprnt.pw/buildStatus/icon?job=docs.supernet.org-updating)](https://jenkinsmaster.sprnt.pw/job/docs.supernet.org-updating/)

---

## Getting started

1) Clone barterDEX and build it:

### Linux/Unix

```
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install git libcurl4-openssl-dev build-essential libnanomsg-dev 
git clone https://github.com/unl0v3d/barterDEX-backend
cd barterDEX-backend/iguana && ./m_mm
```

### OSX
```
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install git libcurl4-openssl-dev build-essential libnanomsg-dev 
git clone https://github.com/unl0v3d/barterDEX-backend
cd barterDEX-backend/iguana && ./m_mm_osx
```

### Windows 

### Running barterDEX

The following steps are not mandatory. After building the barterDEX client "marketmaker" you can launch it up with the required parameter and start using it. We refer to the [barterDEX API documentation](https://docs.kmd.host) at this point.

After successfull compilation install the marketmaker scripts:
```
cd exchanges && ./install
cd ../dexscripts
```
You can find a set of example API call scripts in the current folder `dexscripts`. 

## For android
You have to build a native libnanomsg/libcurl for android. This section is work in progress. Contact ca333@protonmail.ch for assistance on building latest iguana for android.


## Documentation and API ref

You can find the full API reference and introduction guidline at https://docs.kmd.host

