CARLA
=====

This page explains how to use RAMN with CARLA.

About CARLA
-----------

`CARLA <https://carla.org/>`_ is an autonomous driving simulator based on Unreal Engine 4. Users can interact with the simulator using a python API.

Installing CARLA
----------------

Follow these steps to install CARLA on Windows:

* Download and unzip the latest release of CARLA, from their `Github repository <https://github.com/carla-simulator/carla/releases>`_. As of April 2021, the latest version for Windows can be downloaded `here <https://carla-releases.s3.eu-west-3.amazonaws.com/Windows/CARLA_0.9.11.zip>`_.
* Check that the simulator works by launching WindowsNoEditor/CarlaUE4.exe. If a 3D view opens then it means the Unreal Engine part of the simulator is working. If you get an error message, you should download and install the `latest version of Windows DirectX <https://www.microsoft.com/en-us/download/details.aspx?id=35>`_. Make sure you do not let it automatically add a "BING bar" if you do not want it.
* Install Python 3.7 (x64 version) from Python Software foundation's webpage. Make sure you install python 3.7 (i.e. not 3.8 or 3.9) for x86-64 (i.e. not x86). As of April 2021, the latest version can be found `here <https://www.python.org/downloads/release/python-379/>`_.
* Install Python module requirements.
	:code:`$ cd WindowsNoEditor\PythonAPI\examples`
	
	:code:`$ python -m pip install -r requirements.txt`
* Make sure the scripts work by trying some of them.
	:code:`$ python manual_control.py`
	
	:code:`$ python automatic_control.py`
	(automatic_control may require networkx, :code:`$python -m pip install networkx`)
	
* Add CARLA to your python libraries.
	:code:`$ cd WindowsNoEditor\PythonAPI\carla\dist`
	
	:code:`$ python -m easy_install carla-0.9.11-py3.7-win-amd64.egg`
	
* (Optional) You may also download the `Additional maps <https://github.com/carla-simulator/carla/releases>`_. Extract the content of the zip file (e.g. AdditionalMaps_0.9.11.zip) to :code:`<YOUR_CARLA_FOLDER>\WindowsNoEditor` . It should overwrite some files.
* (Optional) `Configure the simulation <https://carla.readthedocs.io/en/stable/configuring_the_simulation/>`_.
	
From there, you should have a functional environment to experiment with CARLA


Installing RAMN scripts
-----------------------

Follow these steps to connect RAMN to CARLA.

* Clone the latest repository of RAMN, or download a zipped version from the `github repository <https://github.com/ToyotaInfoTech/RAMN>`_.
	:code:`$ git clone https://github.com/ToyotaInfoTech/RAMN`
	
* Install the requirements
	:code:`$ python -m pip install -r requirements.txt`
	
Configuring CARLA
-----------------

Please read `CARLA's documentation <https://carla.readthedocs.io/en/latest/adv_rendering_options/>`_ for more information about CARLA's options. The following options are among the most important.

* Specify the quality of graphics using :code:`-quality-level=Epic` (best graphics) or :code:`-quality-level=Low` (best performances).
* Specify the resolution of the server using :code:`-windowed -ResX=N -ResY=N`.
* Specify more settings using an INI file. Refer to `CARLA's page <https://carla.readthedocs.io/en/stable/carla_settings/>`_ for more details. Load a file using the option :code:`-carla-settings="Path/To/CarlaSettings.ini"`.
* If not needed, you may also `disable rendering <https://carla.readthedocs.io/en/latest/adv_rendering_options/>`_.