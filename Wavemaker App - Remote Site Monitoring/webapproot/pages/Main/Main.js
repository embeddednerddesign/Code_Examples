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