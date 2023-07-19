// Diese CRUD-Tabelle ist inspiriert von https://plainenglish.io/blog/basic-html-css-javascript-bootstrap-5-using-external-api-for-crud-operations

// URL des API-Endpunkts zur Abfrage der Anzahl der Objekte
const countUrl = '/api/iRadio/Stations';

// Funktion zum Abrufen der Anzahl der Objekte
function getCount() {
    return fetch(countUrl).then(response => response.json());
}

// URL des API-Endpunkts zur Abfrage der Objektinformationen
function getItemUrl(id) {
    return `/api/iRadio/Station/${id}`;
}

// Funktion zum Abrufen der Objektinformationen
function getItem(id) {
    return fetch(getItemUrl(id)).then(response => response.json());
}

// Funktion zum Aufbau der Tabelle
function buildTable(data) {
    //const sortedData = data.sort((a, b) => a.Station.id - b.Station.id);
    const tbody = document.querySelector('#myTable tbody');

    data.forEach(obj => {
        const tr = document.createElement('tr');
        const td1 = document.createElement('td');
        const td2 = document.createElement('td');
        const td3 = document.createElement('td');
        const td4 = document.createElement('td');
        var foundStation = obj.Station;
        td1.textContent = foundStation.id;
        td2.textContent = foundStation.name;
        td3.textContent = foundStation.url;
        const buttonEdit=document.createElement('button');
        const buttonDel=document.createElement('button');
        const stationIndex=foundStation.id-1;
        buttonEdit.type="button";
        buttonEdit.className="btn btn-outline-secondary";
        buttonEdit.setAttribute("onclick", "showStationEditBox("+stationIndex+")");
        buttonEdit.innerText="Edit";
        buttonDel.type="button";
        buttonDel.className="btn btn-outline-secondary";
        buttonDel.setAttribute("onclick", "stationDelete("+stationIndex+")");
        buttonDel.innerText="Del";
        tr.appendChild(td1);
        tr.appendChild(td2);
        tr.appendChild(td3);
        tr.appendChild(td4);
        td4.appendChild(buttonEdit);
        td4.appendChild(document.createTextNode("\u00A0"));
        td4.appendChild(buttonDel);
        tbody.insertBefore(tr,document.getElementById('last'));
    });
}
var anzStationen;
// Anzahl der Objekte abrufen und Tabelle aufbauen
getCount().then(theAnswer => {
    const promises = [];
    anzStationen = theAnswer.Anzahl;

    // Aufbau der Liste der URLs
    for (let i = 0; i < anzStationen; i++) {
        promises.push(getItem(i));
    }

    const tbody = document.querySelector('#myTable tbody tr');
    tbody.innerHTML = '';

    // Aufruf aller URLs in dem Array promises
    // Wenn das abgeschlossen ist wird buildTable mit dem Ergebnis aufgerufen.
    Promise.all(promises).then(data => {
        buildTable(data);
    });
});

// CRUD-Operationen

// Eingabe-Maske für eine Station einblenden. Es wird die Bibliothek SweetAlert verwendet
function showUserCreateBox() {
    Swal.fire({
        title: "Station",
        html:
            '<input id="Name" class="swal2-input" placeholder="Name">' +
            '<input id="URL" class="swal2-input" placeholder="URL">',
        focusConfirm: false,
        preConfirm: () => {
            stationCreate();
        },
    });
}

function stationCreate() {
    const id = anzStationen++;
    const name = document.getElementById("Name").value;
    const url = document.getElementById("URL").value;

    const xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/api/iRadio/changeStation");
    xhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    xhttp.send(
        JSON.stringify({
            id: id,
            name: name,
            url: url
        })
    );

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            const objects = JSON.parse(this.responseText);
            Swal.fire(objects["status"]);
            buildTable();
        }
    };
}


// Die Daten über den Browser zur Verfügung stellen:

// Funktion zum Herunterladen der Tabelle als JSON-Datei
function downloadTable() {
    let tableData = [];
    let rows = document.querySelectorAll("#myTable tbody tr");
    for (let row of rows) {
        let rowData = {};
        rowData.id = row.cells[0].textContent;
        rowData.name = row.cells[1].textContent;
        rowData.url = row.cells[2].textContent;
        tableData.push(rowData);
    }
    let jsonContent = JSON.stringify(tableData, null, 2);
    let encodedUri = "data:text/json;charset=utf-8," + encodeURIComponent(jsonContent);
    let link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", "myTable.json");
    document.body.appendChild(link);
    link.click();
}

// Funktion zum Hochladen der Tabelle aus einer JSON-Datei
function uploadTable(event) {
    let file = event.target.files[0];
    let reader = new FileReader();
    reader.onload = function (e) {
        let jsonData = e.target.result;
        let tableData = JSON.parse(jsonData);
        let tbody = document.querySelector("#myTable tbody");
        tbody.innerHTML = "";
        for (let rowData of tableData) {
            let newRow = document.createElement("tr");
            newRow.innerHTML = "<td>" + rowData.id + "</td><td>" + rowData.name + "</td><td>" + rowData.url + "</td>";
            tbody.appendChild(newRow);
        }
    }
    reader.readAsText(file);
}


// Ereignisbehandlung für den Download-Button
document.querySelector("#downloadBtn").addEventListener("click", downloadTable);

// Ereignisbehandlung für den Upload-Button
document.querySelector("#uploadBtn").addEventListener("change", uploadTable);

function showStationEditBox(id) {
  console.log(id);
  const xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/api/iRadio/Station/" + id);
  xhttp.send();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      const objects = JSON.parse(this.responseText);
      var foundStation = objects.Station;
      console.log(foundStation);
      Swal.fire({
        title: "Edit Station",
        html:
          '<input id="id" type="hidden" value=' +
          foundStation["id"] +
          ">" +
          '<input id="name" class="swal2-input" placeholder="Name" value="' +
          foundStation["name"] +
          '">' +
          '<input id="url" class="swal2-input" placeholder="URL" value="' +
          foundStation["url"] +
          '">' ,
        focusConfirm: false,
        preConfirm: () => {
          userEdit();
        },
      });
    }
  };
}

function userEdit() {
  const id = document.getElementById("id").value;
  const name = document.getElementById("name").value;
  const url = document.getElementById("url").value;

  const xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/api/iRadio/changeStation");
  xhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
  xhttp.send(
    JSON.stringify({
        id: id,
        name: name,
        url: url
      })
  );
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      const objects = JSON.parse(this.responseText);
      Swal.fire(objects["status"]);
      buildTable();
    }
  };
}

function stationDelete(id) {
    const xhttp = new XMLHttpRequest();
    xhttp.open("DELETE", "/api/iRadio/Station/"+id);
    xhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    xhttp.send();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4) {
        const objects = JSON.parse(this.responseText);
        Swal.fire(objects["status"]);
        buildTable();
      }
    };
  }