// Original version created by TiN (Illya Tsemenko https://xdevs.com/)
// Modified by Shodan (Andrey Bykanov https://misrv.com/)
// ---------------------------------------------------------------

function findGetParameter(parameterName) {
    var result = null,
        tmp = [];
    location.search
        .substr(1)
        .split("&")
        .forEach(function (item) {
          tmp = item.split("=");
          if (tmp[0] === parameterName) result = decodeURIComponent(tmp[1]);
        });
    return result;
}

var logname    = findGetParameter("filename");
$.getScript(logname+'.js', function()
{


var margin = {top: 40, right: 190, bottom: 20, left: 70},
    width = 1550,
    width2 = 1100,
    height = 1050 - margin.top - margin.bottom;
    height2 = 550 - margin.top - margin.bottom;


var group_max=0;
var group_min=0;
var ppm_max=0;
var ppm_min=0;
var min_max_result=[0,0];

var color = d3.scaleOrdinal(d3.schemeCategory10);

var tspan = 0;


var parseDate = d3.timeParse("%d/%m/%Y-%H:%M:%S%Z");

var x = d3.scaleTime().range([0, width]);
//var x = d3.scaleLinear().range([0, width]);

var yw0 = d3.scaleLinear().range([height, 0]); //char A

var ppm = d3.scaleLinear().range([height, 0]); //char A
var ppm_Axis = d3.axisLeft(ppm)  .ticks(axis_tick);

var vertical = d3.select("body")
      .append("div")
      .attr("class", "remove")
      .style("position", "absolute")
      .style("z-index", "19")
      .style("width", "1px")
      .style("height", height+margin.bottom+"px")
      .style("top", "40px")
      .style("bottom", "0px")
      .style("left", "0px")
      .style("background", "#000");
var horizontal = d3.select("body")
      .append("div")
      .attr("class", "remove")
      .style("position", "absolute")
      .style("z-index", "19")
      .style("width", width+2000+"px")
      .style("height", "1px")
      .style("top", "0px")
      .style("bottom", "0px")
      .style("left", "70px")
      .style("background", "#000");

var formatMillisecond = d3.timeFormat(".%L"),
    formatSecond = d3.timeFormat(":%S"),
    formatMinute = d3.timeFormat("%H:%M"),
    formatHour =   d3.timeFormat("%Hh"),
    formatDay =    d3.timeFormat("%d %B"),
    formatWeek =   d3.timeFormat("%d %B"),
    formatMonth =  d3.timeFormat("%d %B"),
    formatYear =   d3.timeFormat("%Y");

function multiFormat(date) {
  return (d3.timeSecond(date) < date ? formatMillisecond
      : d3.timeMinute(date) < date ? formatSecond
      : d3.timeHour(date) < date ? formatMinute
      : d3.timeDay(date) < date ? formatHour
      : d3.timeMonth(date) < date ? (d3.timeWeek(date) < date ? formatDay : formatWeek)
      : d3.timeYear(date) < date ? formatMonth
      : formatYear)(date);
}

var xAxis = d3.axisBottom(x).ticks(20).tickFormat(multiFormat);;



    function NMoveAvg(context, N) {
      this._context = context;
      this._points = {
        x: [],
        y: []
      };
      this._N = N;
    }

    NMoveAvg.prototype = {
      areaStart: function() {
        this._line = 0;
      },
      areaEnd: function() {
        this._line = NaN;
      },
      lineStart: function() {
        this._point = 0;
      },
      lineEnd: function() {
        if (this._line || (this._line !== 0 && this._point === 1)) this._context.closePath();
        this._line = 1 - this._line;
      },
      point: function(x, y) {
        x = +x, y = +y;

        this._points.x.push(x);
        this._points.y.push(y);

        if (this._points.x.length < this._N) return;

        var aX = this._points.x.reduce(function(a, b) {
            return a + b;
          }, 0) / this._N,
          aY = this._points.y.reduce(function(a, b) {
            return a + b;
          }, 0) / this._N;

        this._points.x.shift();
        this._points.y.shift();

        switch (this._point) {
          case 0:
            this._point = 1;
            this._line ? this._context.lineTo(aX, aY) : this._context.moveTo(aX, aY);
            break;
          case 1:
            this._point = 2; // proceed
          default:
            this._context.lineTo(aX, aY);
            break;
        }
      }
    };

    var curveNMoveAge = (function custom(N) {

      function nMoveAge(context) {
        return new NMoveAvg(context, N);
      }

      nMoveAge.N = function(N) {
        return custom(+N);
      };

      return nMoveAge;
    })(0);
// array of curve functions and tites


var svg4 = d3.select("body")
    .append("svg")
    .on("click", function(){  
       mousex = d3.mouse(this);
       vertical.style("left", mousex[0] + 8 + "px" );
       horizontal.style("top", mousex[1] + 6 + "px" );
       })
    .attr("width", width2 + margin.left + 2000)
    .attr("height", height + margin.top + margin.bottom+ 20*20)
    .append("g")
    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");





function make_y_axis() {        
    return d3.axisLeft(yw0)
        .ticks(axis_tick)
}
    
var cnt = 0;

d3.csv(logname)
  .then(function(data) {

    data.splice(0,skip_samples);
    data.splice(cut_samples,9000000);
    data.forEach(function(d) {
        cnt = +(cnt + 1);

        d.date = parseDate(d.date);
//        d.date = +(d.time * 1);

        d.ch1 = +(d.val1 * 1);
        d.ch2 = +(d.val2 * 1);
        d.ch3 = +(d.val3 * 1);
        d.ch4 = +(d.val4 * 1);
        d.ch5 = +(d.val5 * 1);
        d.ch6 = +(d.val6 * 1);
        d.ch7 = +(d.val7 * 1);
        d.ch8 = +(d.val8 * 1);
        d.ch9 = +(d.val9 * 1);
        d.ch10 = +(d.val10 * 1);
        d.ch11 = +(d.val11 * 1);
        d.ch12 = +(d.val12 * 1);
        d.ch13 = +(d.val13 * 1);
        d.ch14 = +(d.val14 * 1);
        d.ch15 = +(d.tecwtt * 1);
        d.ch16 = +(d.tect * 1);

        d.ch17 = +(d.temp1 * 1);
        d.ch18 = +(d.temp2 * 1);
        d.ch19 = +(d.temp3 * 1);
        d.ch20 = +(d.temp4 * 1);
    })
       
    
    // Scale the range of the data
    x.domain([d3.min(data, function(d) { return Math.min(d.date); }), d3.max(data, function(d) { return Math.max(d.date); }) ]);    

    // push a new data point onto the back
    data.push();
    // pop the old data point off the front
    data.shift();
    
svg4.append("g")         
        .attr("class", "grid")
        .call(make_y_axis()
        .tickSize(-width, 0, 0)
        .tickFormat("")
        )

curveArray.forEach(function(daCurve,i) { 

eval(' var yAxis_w_'    + daCurve.channel +' = d3.scaleLinear().range([height, 0])');

if(daCurve.channel=='ch16'||daCurve.channel=='ch17'||daCurve.channel=='ch18'||daCurve.channel=='ch19'||daCurve.channel=='ch20'){
    eval(' var yAxisRight_' + daCurve.channel +' = d3.axisRight(yAxis_w_' + daCurve.channel + ').ticks('+axis_tick+').tickFormat(function(d) {return d3.format(\'.3f\')(d) + \'°C\'})');
} else {
    if(daCurve.axis_is_ppm==0){
	if(daCurve.channel=='ch15'){
	    eval(' var yAxisRight_' + daCurve.channel +' = d3.axisRight(yAxis_w_' + daCurve.channel + ').ticks('+axis_tick+').tickFormat(function(d) {return d3.format(\'.4f\')(d) + \'W\'})');
	} else 
	    if(daCurve.axis_is_exponent==0){
		eval(' var yAxisRight_' + daCurve.channel +' = d3.axisRight(yAxis_w_' + daCurve.channel + ').ticks('+axis_tick+')');
	    }else{
		eval(' var yAxisRight_' + daCurve.channel +' = d3.axisRight(yAxis_w_' + daCurve.channel + ').ticks('+axis_tick+').tickFormat(d3.format(".4e"))');
	    }
    } else {
        eval(' var yAxisRight_' + daCurve.channel +' = d3.axisRight(yAxis_w_' + daCurve.channel + ').ticks('+axis_tick+').tickFormat(function(d) {return d3.format(\'.2f\')(((d/d3.median(data, function(d) { return (d[daCurve.channel]);} ).toPrecision(7))-1)*1E6) + \' ppm\'})');
    }
}

if(daCurve.tspan>0){

    var max_temp= d3.extent(data, function(d) { return d[daCurve.channel]; })[1];
    var min_temp= d3.extent(data, function(d) { return d[daCurve.channel]; })[0];

    tspan = max_temp - min_temp;
}

curveArray.forEach(function(daCurve_group,i) { 

    if(daCurve_group.group>0){

	var max= d3.extent(data, function(d) { return d[daCurve_group.channel]; })[1];
	var min= d3.extent(data, function(d) { return d[daCurve_group.channel]; })[0];
        if(group_min==0){group_min=min;};

        if(max>group_max){group_max=max;};
	if(min<group_min){group_min=min;};
    } 
        min_max_result = [group_min, group_max];

        var diff = group_max - group_min;

	ppm_max=diff/(((group_max+group_min)/2)/1E6)/2;
	ppm_min=-ppm_max;
	ppm.domain([ppm_min,ppm_max]);

});


if(daCurve.group==0){
    var max= d3.extent(data, function(d) { return d[daCurve.channel]; })[1];
    var min= d3.extent(data, function(d) { return d[daCurve.channel]; })[0];
    var diff = (((max - min)/100)*daCurve.scale);

min_max_result = [min-diff, max+diff];
if(daCurve.offset>0)min_max_result = [min-diff-daCurve.offset, max+diff];
if(daCurve.offset<0)min_max_result = [min-diff, max+diff-daCurve.offset];
}

//console.log(min_max_result);
//console.log([ppm_min,ppm_max]);
eval(' yAxis_w_' + daCurve.channel +'.domain(min_max_result)');


eval('svg4.append("path")'+
	'.datum(data)'+
	'.style("stroke",  function() {return daCurve.color = color(daCurve.curveTitle); })'+
	'.style("stroke-width", "2px").style("opacity", line_op)'+
	'.attr("data-legend-pos", 1).attr("data-legend",function(d) { return \' + daCurve.curveTitle + \' })'+
	'.attr("d",  d3.line()'+
		'.x(function(d) {       return   x(d.date); })'+
		'.y(function(d) {       return yAxis_w_'+daCurve.channel+'(d[daCurve.channel]); })'+
		'.defined(function(d) { return     d[daCurve.channel]; })'+
		'.curve(curveNMoveAge.N(avgSamples)))');


eval('svg4.selectAll("dot")'+
        '.data(data)'+
        '.enter().append("circle")'+
        '.style("fill", function() {return daCurve.color = color(daCurve.curveTitle); })'+
        '.style("opacity", circle_op)'+
        '.attr("r", circle_size)'+
        '.attr("cx", function(d) { return x(d.date); })'+
        '.attr("cy", function(d) { return yAxis_w_'+daCurve.channel+'(d[daCurve.channel]); })');


    var max= d3.extent(data, function(d) { return d[daCurve.channel]; })[1];
    var min= d3.extent(data, function(d) { return d[daCurve.channel]; })[0];
    var diff = max - min;

if(daCurve.channel=='ch17'||daCurve.channel=='ch18'||daCurve.channel=='ch19'||daCurve.channel=='ch20'){
    svg4.append("text")      // text label for the x axis
        .attr("x", 50 )
        .attr("y",  height + margin.top + margin.bottom+ (20 * i) )
        .style("text-anchor", "left")
        .style("fill", function() {return daCurve.color = color(daCurve.curveTitle); })
        .style("font-size","18px")
        .text(daCurve.curveTitle + " MEDIAN: " + d3.median(data, function(d) { return (d[daCurve.channel]);} ).toFixed(3) + "°C " +
                                   "σ=" + (d3.deviation(data, function(d) { return (d[daCurve.channel]);} )*1e3).toFixed(3) + "m°C " +
                                   "Peak-to-peak: " + (max-min).toFixed(3) + "°C");
} else  if(daCurve.channel=='ch16'){
    svg4.append("text")      // text label for the x axis
        .attr("x", 50 )
        .attr("y",  height + margin.top + margin.bottom+ (20 * i) )
        .style("text-anchor", "left")
        .style("fill", function() {return daCurve.color = color(daCurve.curveTitle); })
        .style("font-size","18px")
        .text(daCurve.curveTitle + " MEDIAN: " + d3.median(data, function(d) { return (d[daCurve.channel]);} ).toFixed(2) + "°C " +
                                   "Peak-to-peak: " + (max-min).toFixed(2) + "°C");
} else if(daCurve.channel=='ch15'){
    svg4.append("text")      // text label for the x axis
        .attr("x", 50 )
        .attr("y",  height + margin.top + margin.bottom+ (20 * i) )
        .style("text-anchor", "left")
        .style("fill", function() {return daCurve.color = color(daCurve.curveTitle); })
        .style("font-size","18px")
        .text(daCurve.curveTitle + " MEDIAN: " + d3.median(data, function(d) { return (d[daCurve.channel]);} ).toFixed(2) + "W " +
                                   "Peak-to-peak: " + (max-min).toFixed(2) + "W");
} else {
    svg4.append("text")      // text label for the x axis
        .attr("x", 50 )
        .attr("y",  height + margin.top + margin.bottom+ (20 * i) )
        .style("text-anchor", "left")
        .style("fill", function() {return daCurve.color = color(daCurve.curveTitle); })
        .style("font-size","18px")
        .text(daCurve.curveTitle + " MEDIAN: " + d3.median(data, function(d) { return (d[daCurve.channel]);} ).toFixed(17) + " " +
                                   "σ=" + (d3.deviation(data, function(d) { return (d[daCurve.channel]);} )*1e6).toFixed(3) + "u " +
                                   "Peak-to-peak: " + ((diff/(((max+min)/2)/1E6))).toFixed(3) + "ppm");
}

eval('svg4.append("g")' +
	'.attr("class", "y axis")' +
	'.attr("stroke", function() {return daCurve.color = color(daCurve.curveTitle);})' +
	'.attr("transform", "translate(" + (width+(100*i)) +" ,0)")' +
	'.style("font-size","16px")' +
	'.call(yAxisRight_'+daCurve.channel+')');

});

svg4.append("text")      // text label for the x axis
        .attr("x", 150 )
        .attr("y",  -10 )
        .style("text-anchor", "left")
        .style("fill","#035")
        .style("font-size","24px")
        .text(logname+"      SMPL:" + cnt + " AVG:"+avgSamples+"      ΔT:" + tspan.toFixed(3) + "°C");

svg4.append("text")      // text label for the x axis
        .attr("x", -50 )
        .attr("y",  -10 )
        .style("text-anchor", "right")
        .style("fill","#000000")
        .style("font-weight","bold")
        .style("font-size","16px")
        .text("PPM");

svg4.append("g")            // Add the PPM Axis
        .attr("class", "y axis")
        .attr("transform", "translate(0 ,0)")
        .style("fill", "#000000")
        .style("font-size","18px")
        .call(ppm_Axis);

svg4.append("g")            // Add the X Axis
        .attr("class", "x axis")
        .attr("transform", "translate(0," + height + ")")
        .style("font-size","12px")
        .call(xAxis);

});
});
