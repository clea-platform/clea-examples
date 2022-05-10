# SECO-PoC-FaceAgeEmotionDetection

There are 2 versions:
1) The first run with [deepface](https://github.com/serengil/deepface) libraries. 
2) The second one with [openVino](https://www.intel.com/content/www/us/en/developer/tools/openvino-toolkit/overview.html) toolkit 

The reason is that openVino runs only on Intel processors.
We prefer/suggest to use openVino in production because it is much faster than deepface that lands on tensorflow.
 
## Installation
We used and tested with python 3.8.

In both version, it must be installed the libraries in requirements.txt; with the command:
```text
pip install -r requirements.txt
```

#### Deepface
If you want to use deepface (or you don't have an Intel processor), it's necessary also:
```text
pip install -r requirements_deepface.txt
```
<details>
  <summary>NOTE</summary>
Please be careful with tensorflow libraries, sometimes you need to install a different version or download
a pre-compiled wheel.
We suggest to use anaconda.
</details>

#### OpenVino
For openVino you need to follow the installation from this [link](https://docs.openvino.ai/latest/openvino_docs_install_guides_install_runtime.html).


## Run
Before to run main.py, Astarte needs to set the following variable:
```python
device_id = 'example: 2-ASUSOCIwSERS-GWLA-Xg'
realm = 'example: devenv0'
credentials_secret = 'example: 1YZEuEGLKlD57UwDG7lmE7yHXDlrC4LEzdiS594HUEI='
pairing_base_url = "example: https://api.demo.clea.cloud/pairing"
```
In order to get the previous credentials, please follow the Astarte [documentation](https://docs.astarte-platform.org/latest/001-intro_user.html) 

To simplify, you need to write the credentials in "**_`astarte/as_conn.py`_**" (line 23).

<details>
  <summary>NOTE</summary>
The application is not responsive. It is designed for 1080*1920 monitor. So you need to prepare a vertical monitor.
</details>

#### Deepface
If the installation of the prerequisites was successful, run the following command:
```text
python main.py
```

#### OpenVino
In MainWindow.py, please uncomment line 7 (`from VideoThread_openVino import VideoThread`) 
and comment line 6 (`from VideoThread import VideoThread`).
Furthermore, you need to activate some openVino environment variables.

In Linux and macOS is:
```text
source  <INSTALL_DIR>/setupvars.sh
```
instead, Windows is:
```text
<INSTALL_DIR>\setupvars.bat
```
https://docs.openvino.ai/latest/openvino_docs_get_started_get_started_demos.html.

Finally:
```text
python main.py
```
