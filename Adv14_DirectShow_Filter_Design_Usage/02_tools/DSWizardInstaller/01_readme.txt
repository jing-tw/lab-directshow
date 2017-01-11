support for Visual studio 2005
							by Jing

<<Install>>
Step 1: DSWizardInstaller.exe
Step 2: copy  .\DSWizard.vsz C:\Program Files\Microsoft Visual Studio 8\VC\vcprojects


Note: 
===============DSWizard.vsz ============
VSWIZARD 7.0
Wizard=VsWizard.VsWizardEngine.8.0
Param="WIZARD_NAME = DSWizard"
Param="RELATIVE_PATH=VCWizards"
Param="WIZARD_UI=TRUE"


<<Path>>
C/C++:Additional Include Directories:
	$(DSHOWBASECLASSES)\.;$(DXSDK)\include;C:\Program Files\Microsoft SDKs\Windows\v6.0\Include

Linker:Additional Library Directories:
	$(DSHOWBASECLASSES)\Debug;$(DXSDK)\lib;C:\Program Files\Microsoft SDKs\Windows\v6.0\Lib

<<.def>>
重新加入