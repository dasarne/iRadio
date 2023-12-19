const char index_html[] PROGMEM = R"rawliteral(
<html><head>
    <title>Network Setup</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body,
        html {
            height: 100%;
            line-height: 1.8;
        }
        .btn {
            background-color: white;
            border: 2px solid black;
            color: black;
            padding: 10px 24px;
            cursor: pointer;
            width: fit-content;
            border-radius: 3%;
            margin: 2px;
        }
        .btn:hover {
            background: #e7e7e7;
        }
        .btn2 {
            background-color: lightgray;
            border: 2px solid black;
            color: black;
            padding: 10px 24px;
            cursor: pointer;
            width: fit-content;
            border-radius: 3%;
            margin: 2px;
        }
        .btn2:hover {
            background: whitesmoke;
        }
    </style>
</head>
<body>
    <h1>Network Setup</h1> <span id="Networks">
    <!--networks-->
    <br>
    <button class="btn2" onclick="wifi()">Add Network</button>
    </span>
</body>
<script>
    function refresh() {
        setTimeout(function () {
            location.reload();
        }, 3000);
    }

    function passphrase(button) {
        let passphrase = prompt('Please enter the passphrase for ' + button.textContent + ".");
        sendCredentials(button.textContent, passphrase);
    }

    function wifi() {
        let SSID = prompt('Please enter the SSID for the network you want to connect to.');
        let passphrase = prompt('Please enter the passphrase for ' + SSID + ".");
        sendCredentials(SSID, passphrase);
    }

    function sendCredentials(ssid, password) {
        if (ssid == null || password == null) {
            message("Please enter a valid SSID and password. Getting networks..<br>");
            refresh();
            return;
        } else {
            message("Trying to connect to " + ssid + "..<br>");
            let xhttp = new XMLHttpRequest();
            xhttp.open("GET", "connect?ssid=" + ssid + "&password=" + password, true);
            xhttp.setRequestHeader("ssid", ssid);
            xhttp.setRequestHeader("passphrase", password);
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    if (this.responseText.startsWith("SUCCESS")) {
                        let url = "http://" + this.responseText.substring(8);
                        message("&#x2705; Successfully connected to " + ssid + ". <br> Please connect to your main network and click the button below to connect to this address: <br> " + url + "<br>" + "<button class=\"btn\" onclick=\"window.location.href='" + url + "'\">Connect</button>") + "<br>";  
                    } else {
                        message("&#x274C; Failed to connect to " + ssid + ". Getting networks.. <br>");
                        refresh();
                    }
                }
            };
            xhttp.send();
        }
    }

    function message(message) {
        document.getElementById("Networks").innerHTML = message;
    } 
</script>
</html>
)rawliteral";

const char list_html[] PROGMEM = R"rawliteral(
 <html>

<head>
    <title>Management</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body,
        html {
            height: 100%;
            line-height: 1.8;
        }
        button {
            margin-left: 10px;
        }
    </style>
</head>

<body>
    <h1>Saved Networks</h1>
    <span id="Networks">   
        <ul id="Network-list">
            <!--savednetworks-->
        </ul>
    </span>
</body>
<script>
    function refresh() {
        //wait 5 seconds
        setTimeout(function () {
            //refresh page
            location.reload();
        }, 3000);
    }

    function deleteNetwork(ssid, index) {
        //ask for confirmation
        if (confirm("Are you sure you want to delete " + ssid + "?")) {
            //send delete request
            deleteRequest(ssid, index);
        }
    }

    function deleteRequest(ssid, index) {
        message("&#x231B; Deleting " + ssid + "... <br>");
        //send delete request
        let xhttp = new XMLHttpRequest();
        xhttp.open("GET", "delete?index=" + ssid, true);
        xhttp.setRequestHeader("index", index);
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                if (this.responseText.startsWith("SUCCESS")) {
                    message("&#x2705; Successfully deleted " + ssid + ". Getting networks.. <br>");
                    refresh();
                } else {
                    message("&#x274C; Failed to delete " + ssid + ". Getting networks.. <br>");
                    refresh();
                }
            }
        };
        xhttp.send();
    }
    
    function message(message) {
        document.getElementById("Networks").innerHTML = message;
    }
</script>

</html> 
)rawliteral";