dojo.declare("SiteStatusMonitorPDC", wm.Application, {
	"disableDirtyEditorTracking": false, 
	"eventDelay": 0, 
	"hintDelay": 1500, 
	"i18n": false, 
	"isSecurityEnabled": false, 
	"main": "Main", 
	"manageHistory": true, 
	"manageURL": false, 
	"name": "", 
	"phoneGapLoginPage": "Login", 
	"phoneMain": "", 
	"projectSubVersion": "Alpha11", 
	"projectVersion": 1, 
	"sessionExpirationHandler": "navigateToLogin", 
	"studioVersion": "6.7.0.RELEASE", 
	"tabletMain": "", 
	"theme": "wm.base.widget.themes.wm_default", 
	"toastPosition": "br", 
	"touchToClickDelay": 500, 
	"touchToRightClickDelay": 1500,
	"widgets": {
		silkIconList: ["wm.ImageList", {"colCount":39,"height":16,"iconCount":90,"url":"lib/images/silkIcons/silk.png","width":16}, {}], 
		siteLocations: ["wm.Variable", {"isList":true,"json":"[{\"dataValue\":\"Phoenix Dream Center (PDC)\"},{\"dataValue\":\"University of Nations (UoN)\"}]","type":"StringData"}, {}]
	},
	_end: 0
});

SiteStatusMonitorPDC.extend({

	_end: 0
});