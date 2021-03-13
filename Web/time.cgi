t <html>
t <head><title>LED Control</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var timeUpdate = new periodicObj("time_rtc.cgx", 1000);
t function renderTime() {
t 	adVal = document.getElementById("time_rtc").value;
t }
t function periodicTimeUpdate() {
t   updateMultiple(timeUpdate, renderTime);
t   ad_elTime = setTimeout(periodicTimeUpdate, timeUpdate.period);
t }
t periodicTimeUpdate();
t </script></head>
i pg_header.inc
t <h2 align=center><br>µC Timing</h2>
t <div>
t 	<p><font size="2">This page shows the time kept by the µC</p>
t 	<ol>
t 		<li>The time kept by the <b>Real Time Clock (RTC)</b> at request time.</li>
t 		<li>The time when the last <b>SNTP Request</b> was made by the µC</li></font>
t 	</ol>
t </div>
t <form action=time.cgi method=post name=form1>
t 	<input type=hidden value="led" name=pg>
t 	<table border=0 width=99%><font size="3">
t 		<tr bgcolor=#aaccff>
t  			<th width=40%>Item</th>
t  			<th width=60%>Value</th>
t 		</tr>
t 		<tr>
t 			<td><img src=pabb.gif>µC RTC Date:</td>
t 			<td><input type="text" readonly style="background-color: transparent; border: 0px"
c t m   	size="30" id="time_rtc" value="%s"></td>
t 		</tr>
t 		<tr>
t 			<td><img src=pabb.gif>Last SNTP Request:</td>
t 			<td><input type="text" readonly style="background-color: transparent; border: 0px"
c t n   	size="30" id="time_sntp" value="%s"></td>
t 		</tr>
t 	</table>
t </form>
i pg_footer.inc
. End of script must be closed with period.
