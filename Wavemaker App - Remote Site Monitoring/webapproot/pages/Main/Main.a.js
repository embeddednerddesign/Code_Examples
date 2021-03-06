dojo.declare("Main", wm.Page, {
start: function() {
this.UpdateSiteStatus();
},
"preferredDevice": "desktop",
UpdateSiteStatus: function(inSender) {
var xmlhttp;
var url;
var genData;
var lightRed = "#FF3333";
var goodFontColor = "black";
var badFontColor = lightRed;
var goodBackgroundColor = "lightgreen";
var badBackgroundColor = lightRed;
var goodCaption = "ok";
var badCaption = "warning";
var sitePrefix = "us.az.phoenix.phoenixdreamcenter";
// this url is used to request a full set of data from the database
url = "http://bldash.us/services/q?type=values&options=regex&path="+sitePrefix+".*";
var numberOfGenerators = 24;
var numberOfTrackersPerGenerator = 4;
var deltaAzimuthLimit = 20;
var deltaElevationLimit = 10;
// monitors.name is the name of the cells in the table
// monitors.dataValue is the key name for the database response
var monitors = [
{"name":"PumpAlarm*","dataValue":sitePrefix+".generator.g**.alarm.coolant"},
{"name":"PermissiveAlarm*","dataValue":sitePrefix+".generator.g**.alarm.permissive"},
{"name":"WindAlarm*","dataValue":sitePrefix+".generator.g**.alarm.wind"},
{"name":"T1ActiveAlarm*","dataValue":sitePrefix+".generator.g**.trackers.t1.alarm.active"},
{"name":"T2ActiveAlarm*","dataValue":sitePrefix+".generator.g**.trackers.t2.alarm.active"},
{"name":"T3ActiveAlarm*","dataValue":sitePrefix+".generator.g**.trackers.t3.alarm.active"},
{"name":"T4ActiveAlarm*","dataValue":sitePrefix+".generator.g**.trackers.t4.alarm.active"},
{"name":"T1DeltaAz*","dataValue":sitePrefix+".generator.g**.trackers.t1.azimuth"},
{"name":"T2DeltaAz*","dataValue":sitePrefix+".generator.g**.trackers.t2.azimuth"},
{"name":"T3DeltaAz*","dataValue":sitePrefix+".generator.g**.trackers.t3.azimuth"},
{"name":"T4DeltaAz*","dataValue":sitePrefix+".generator.g**.trackers.t4.azimuth"},
{"name":"T1DeltaEl*","dataValue":sitePrefix+".generator.g**.trackers.t1.elevation"},
{"name":"T2DeltaEl*","dataValue":sitePrefix+".generator.g**.trackers.t2.elevation"},
{"name":"T3DeltaEl*","dataValue":sitePrefix+".generator.g**.trackers.t3.elevation"},
{"name":"T4DeltaEl*","dataValue":sitePrefix+".generator.g**.trackers.t4.elevation"}
];
if (window.XMLHttpRequest)
{// code for IE7+, Firefox, Chrome, Opera, Safari
xmlhttp=new XMLHttpRequest();
}
else
{// code for IE6, IE5
xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
}
xmlhttp.onreadystatechange=function()
{
if (xmlhttp.readyState==4 && xmlhttp.status==200)
{
// initialize a variable to hold the parsed response. One array of values for key.
var parsedResponse = [
{
"PumpAlarm":[],
"PermissiveAlarm":[],
"WindAlarm":[],
"T1ActiveAlarm":[],
"T2ActiveAlarm":[],
"T3ActiveAlarm":[],
"T4ActiveAlarm":[],
"T1DeltaAz":[],
"T2DeltaAz":[],
"T3DeltaAz":[],
"T4DeltaAz":[],
"T1DeltaEl":[],
"T2DeltaEl":[],
"T3DeltaEl":[],
"T4DeltaEl":[],
"SolarAz":[],
"SolarEl":[]
}
];
// load the response
genData = xmlhttp.responseText;
// make it a javascript object
genDataObj = JSON.parse(genData);
// *********************************************************
// update the monitor values in the parsedResponse variable
// *********************************************************
// create an array of values for each generator
for (var i = 0; i < numberOfGenerators; i++)
{
// code logic is zero-referenced but generators start at 1
var generatorNumber = i+1;
// add a new value to this generator's array for each monitor
for (var j = 0; j < monitors.length; j++)
{
// the name of each monitor(row in table) in the data structure is without the "*"
var monitorName = monitors[j].name.replace("*","");
// update the monitor keys accordingly to match the geneator number
if (generatorNumber < 10)
{
// it's a single digit generator, so replace the first "*" with a zero
newMonitorKey = monitors[j].dataValue.replace("g*","g0");
// replace the second "*" with the generator number
newMonitorKey = newMonitorKey.replace("*",generatorNumber);
}
else
{
// it's a two-digit generator, so replace both "*"s with the generator number
newMonitorKey = monitors[j].dataValue.replace("**",generatorNumber);
}
try
{
// if the key is in the response, add it's value to the array
parsedResponse[0][monitorName].push(genDataObj[newMonitorKey][1]);
}
catch(err)
{
// if the key is NOT in the response, put "N/A" in place of the data
parsedResponse[0][monitorName].push("N/A");
}
}
}
// since there is not a sun. value for each generator, we handle this separately
parsedResponse[0]["SolarAz"] = genDataObj[sitePrefix+".sun.azimuth"][1];
parsedResponse[0]["SolarEl"] = genDataObj[sitePrefix+".sun.elevation"][1];
// *********************************************************
// update the grid from the parsedResponse variable
// *********************************************************
// cycle through each generator, updating its table values and formatting
for (var i = 0; i < numberOfGenerators; i++)
{
// this is a flag used to tell if a given generator had any faults
var generatorProblem = false;
// code logic is zero-referenced but generators start at 1
var generatorNumber = i+1;
// add a new value to this generator's array for each monitor
for (var j = 0; j < monitors.length; j++)
{
// the name of each monitor(row in table) in the data structure is without the "*"
var monitorName = monitors[j].name.replace("*","");
// update the monitor keys accordingly to match the geneator number
if (generatorNumber < 10)
{
// it's a single digit generator, so replace the first "*" with a zero
newMonitorKey = monitors[j].dataValue.replace("g*","g0");
// replace the second "*" with the generator number
newMonitorKey = newMonitorKey.replace("*",generatorNumber);
}
else
{
// it's a two-digit generator, so replace both "*"s with the generator number
newMonitorKey = monitors[j].dataValue.replace("**",generatorNumber);
}
// format the cells of the table accordingly
try
{
var labelName = monitors[j].name.replace("*",generatorNumber);
// check to see if we are working with the Azimuth/Elevation calcs
if (labelName.search("Delta") == -1)
{
// we are NOT working with the Azimuth/Elevation calcs
// check to see if we are working on the Tracker Active Alarms
if (labelName.search("ActiveAlarm") == -1)
{
// we are NOT working with the Tracker Active Alarms
if (parsedResponse[0][monitorName][i] == 0)
{
// everything is ok, so set it that way
wm.Page.getPage("Main")[labelName].setValue("caption",goodCaption);
// make the font black
wm.Page.getPage("Main")[labelName].setStyle("color",goodFontColor);
}
else
{
// there is an alarm, so set it appropriately
generatorProblem = true;
wm.Page.getPage("Main")[labelName].setValue("caption",badCaption);
// make the font red
wm.Page.getPage("Main")[labelName].setStyle("color",badFontColor);
}
}
else
{
// we are working with the Active Alarms
if (parsedResponse[0][monitorName][i] == 0)
{
// everything is ok, so set it that way
wm.Page.getPage("Main")[labelName].setValue("caption",goodCaption);
// make the font black
wm.Page.getPage("Main")[labelName].setStyle("color",goodFontColor);
}
else
{
// there is a non-zero active alarm code, so display it appropriately
generatorProblem = true;
wm.Page.getPage("Main")[labelName].setValue("caption",parsedResponse[0][monitorName][i]);
// make the font red
wm.Page.getPage("Main")[labelName].setStyle("color",badFontColor);
}
}
}
else
{
// we are working with the azimuth/elevation calcs
// calculate the absolute value of the difference between the tracker azimuth and the solar azimuth
azValue = Math.abs(parsedResponse[0][monitorName][i] - parsedResponse[0]["SolarAz"]).toFixed(1);
// the azimuth and elevation values are numberOfTrackersPerGenerator apart in the data array, so adjust accordingly in order to grab the elevation value
j += numberOfTrackersPerGenerator;
monitorName = monitors[j].name.replace("*","");
// calculate the absolute value of the difference between the tracker elevation and the solar elevation
elValue = Math.abs(parsedResponse[0][monitorName][i] - parsedResponse[0]["SolarEl"]).toFixed(1);
// check if we have reached the end of the azimuth/elevation calcs by looking for the final tracker number in the monitorName
if (monitorName.search("T"+numberOfTrackersPerGenerator) == -1)
{
// we didn't find the final tracker number in monitor name so we're not done yet. Flip the counter back to grab the next azimuth value
j -= numberOfTrackersPerGenerator;
}
// check if either value has become a NaN after the math calcs, and if so, make them "N/A"
if (isNaN(azValue))
azValue = "N/A";
if (isNaN(elValue))
elValue = "N/A";
// fill the table cell with the azimuth delta value followed by the elevation delta value
wm.Page.getPage("Main")[labelName].setValue("caption",azValue + "," + elValue);
// check if either of the values have exceeded their limits
if ((azValue > deltaAzimuthLimit) || (elValue > deltaElevationLimit))
{
// at least one of the values is problematic, so set it accordingly
generatorProblem = true;
// make the font red
wm.Page.getPage("Main")[labelName].setStyle("color",badFontColor);
}
}
// check if there were any problems with this generator
if (generatorProblem == true)
{
// there were problems, so make the background red
wm.Page.getPage("Main")["Generator"+generatorNumber].setBackgroundColor(badBackgroundColor);
}
else
{
// there were no problems, so make the background white
wm.Page.getPage("Main")["Generator"+generatorNumber].setBackgroundColor(goodBackgroundColor);
}
}
catch(err){}
}
}
//                wm.Page.getPage("Main").varStatusResp.setData(parsedResponse);
/*
for (var k = 0; k < testVar.length; k++)
{
for (var l = 0; l < testVar[k].dataValue.length; l++)
{
console.log(testVar[k].dataValue[l]);
}
}
*/
}
}
xmlhttp.open("GET",url,true);
xmlhttp.send();
},
_end: 0
});

Main.widgets = {
timerUpdateSiteStatus: ["wm.Timer", {"delay":60000}, {"onTimerFire":"UpdateSiteStatus"}],
varStatusResp: ["wm.Variable", {"isList":true,"json":"[]","type":"EntryData"}, {}],
layoutBox1: ["wm.Layout", {"autoScroll":false,"horizontalAlign":"left","styles":{},"verticalAlign":"top"}, {}, {
panel1: ["wm.MainContentPanel", {"height":"120px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
BlLogo: ["wm.Picture", {"height":"120px","source":"resources/images/BLPlogoweb3.png","width":"325px"}, {}],
label1Panel: ["wm.Panel", {"height":"112px","horizontalAlign":"left","verticalAlign":"bottom","width":"230px"}, {}, {
label1: ["wm.Label", {"caption":"Site Status Monitor","height":"69px","padding":"4","styles":{"fontSize":"24px","fontFamily":"Arial","fontWeight":"normal"},"width":"100%"}, {}]
}],
siteLocationListPanel: ["wm.Panel", {"height":"200px","horizontalAlign":"left","styles":{},"verticalAlign":"bottom","width":"100%"}, {}]
}],
panel254: ["wm.Panel", {"border":"2","height":"2px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}],
label16PanelPanel: ["wm.Panel", {"height":"100%","horizontalAlign":"left","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
label16Panel: ["wm.MainContentPanel", {"border":"0","height":"36px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
spacer73: ["wm.Spacer", {"height":"100%","width":"48px"}, {}],
siteName: ["wm.Label", {"caption":"Phoenix Dream Center","height":"36px","padding":"4","styles":{"fontSize":"22px"},"width":"100%"}, {}]
}],
panel259: ["wm.Panel", {"border":"1","height":"2px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}],
panel2: ["wm.MainContentPanel", {"height":"100%","horizontalAlign":"center","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
panel3: ["wm.Panel", {"autoScroll":true,"height":"100%","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
spacer13: ["wm.Spacer", {"height":"100%","styles":{},"width":"48px"}, {}],
rowLabels: ["wm.Panel", {"height":"100%","horizontalAlign":"left","styles":{},"verticalAlign":"top","width":"100px"}, {}, {
labelGeneratorNumber1: ["wm.Label", {"align":"center","caption":"Generator #","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
labelPumpAlarm1: ["wm.Label", {"align":"left","border":"1,1,1,0","borderColor":"#070000","caption":"Pump","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelPermissiveAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"Permissive","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelWindAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"Wind","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT1ActiveAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T1-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT2ActiveAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T2-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT3ActiveAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T3-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT4ActiveAlarm1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T4-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT1DeltaAz1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T1-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT2DeltaAz1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T2-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT3DeltaAz1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T3-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT4DeltaAz1: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T4-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber1: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator1: ["wm.Label", {"align":"center","caption":"1","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm1: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","display":undefined,"height":"100%","margin":"0,0,0,0","padding":"4","styles":{"borderRadius":"0px"},"width":"100%"}, {}],
PermissiveAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz1: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber2: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator2: ["wm.Label", {"align":"center","caption":"2","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm2: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz2: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber3: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator3: ["wm.Label", {"align":"center","caption":"3","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm3: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz3: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber4: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator4: ["wm.Label", {"align":"center","caption":"4","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm4: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz4: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber5: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator5: ["wm.Label", {"align":"center","caption":"5","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm5: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz5: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber6: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator6: ["wm.Label", {"align":"center","caption":"6","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm6: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz6: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber7: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator7: ["wm.Label", {"align":"center","caption":"7","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm7: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz7: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber8: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator8: ["wm.Label", {"align":"center","caption":"8","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm8: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz8: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber9: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator9: ["wm.Label", {"align":"center","caption":"9","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm9: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz9: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber10: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator10: ["wm.Label", {"align":"center","caption":"10","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm10: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz10: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber11: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator11: ["wm.Label", {"align":"center","caption":"11","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm11: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz11: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber12: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator12: ["wm.Label", {"align":"center","caption":"12","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm12: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","margin":"0,0,0,0","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz12: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}]
}],
spacer19: ["wm.Spacer", {"height":"4%","styles":{},"width":"100%"}, {}],
panel255: ["wm.Panel", {"border":"1","height":"2px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}],
panel74: ["wm.Panel", {"autoScroll":true,"height":"100%","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}, {
spacer14: ["wm.Spacer", {"height":"100%","styles":{},"width":"48px"}, {}],
rowLabels1: ["wm.Panel", {"height":"100%","horizontalAlign":"left","styles":{},"verticalAlign":"top","width":"100px"}, {}, {
labelGeneratorNumber2: ["wm.Label", {"align":"center","caption":"Generator #","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
labelPumpAlarm2: ["wm.Label", {"align":"left","border":"1,1,1,0","borderColor":"#070000","caption":"Pump","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelPermissiveAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"Permissive","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelWindAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"Wind","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT1ActiveAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T1-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT2ActiveAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T2-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT3ActiveAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T3-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT4ActiveAlarm2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T4-Active Alarm","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT1DeltaAz2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T1-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT2DeltaAz2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T2-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT3DeltaAz2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T3-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
labelT4DeltaAz2: ["wm.Label", {"align":"left","border":"0,1,1,0","borderColor":"#070000","caption":"T4-Delta Az,El","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber13: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator13: ["wm.Label", {"align":"center","caption":"13","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm13: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz13: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber14: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator14: ["wm.Label", {"align":"center","caption":"14","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm14: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz14: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber15: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator15: ["wm.Label", {"align":"center","caption":"15","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm15: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz15: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber16: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator16: ["wm.Label", {"align":"center","caption":"16","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm16: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz16: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber17: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator17: ["wm.Label", {"align":"center","caption":"17","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm17: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz17: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber18: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator18: ["wm.Label", {"align":"center","caption":"18","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm18: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz18: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber19: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator19: ["wm.Label", {"align":"center","caption":"19","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm19: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz19: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber20: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator20: ["wm.Label", {"align":"center","caption":"20","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm20: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz20: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber21: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator21: ["wm.Label", {"align":"center","caption":"21","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm21: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz21: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber22: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator22: ["wm.Label", {"align":"center","caption":"22","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm22: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz22: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber23: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator23: ["wm.Label", {"align":"center","caption":"23","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm23: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz23: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}],
genNumber24: ["wm.Panel", {"height":"100%","horizontalAlign":"left","verticalAlign":"top","width":"70px"}, {}, {
Generator24: ["wm.Label", {"align":"center","caption":"24","height":"100%","padding":"4","styles":{"fontSize":"12px","fontWeight":"bold"},"width":"100%"}, {}],
PumpAlarm24: ["wm.Label", {"align":"center","border":"1,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
PermissiveAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
WindAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1ActiveAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2ActiveAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3ActiveAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4ActiveAlarm24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T1DeltaAz24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T2DeltaAz24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T3DeltaAz24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}],
T4DeltaAz24: ["wm.Label", {"align":"center","border":"0,1,1,0","borderColor":"#070000","caption":"","height":"100%","padding":"4","styles":{},"width":"100%"}, {}]
}]
}],
spacer20: ["wm.Spacer", {"height":"4%","styles":{},"width":"100%"}, {}],
panel257: ["wm.Panel", {"border":"2","height":"4px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}],
spacer21: ["wm.Spacer", {"height":"4%","styles":{},"width":"100%"}, {}],
panel258: ["wm.Panel", {"border":"2","height":"4px","horizontalAlign":"left","layoutKind":"left-to-right","styles":{},"verticalAlign":"top","width":"100%"}, {}]
}]
}]
}]
};

Main.prototype._cssText = '';
Main.prototype._htmlText = '';