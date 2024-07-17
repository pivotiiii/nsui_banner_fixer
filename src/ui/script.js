const getElements = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const ui = getElements(["filesTableBody", "addBtn", "fixBtn", "tableContainer", "hr1", "hr2", "checkboxDiv", "checkboxReplace"]);

ui.addBtn.addEventListener("click", async () => {
    ui.addBtn.classList.remove("app-btn-primary");
    const buttonInnerHTMLBackup = ui.addBtn.innerHTML;
    ui.addBtn.innerHTML = '<div class="loader" style="border: 3px solid #60CDFF;"></div>';
    let values = await window.add_cias();
    buildTable(values);
    ui.addBtn.innerHTML = buttonInnerHTMLBackup;
})

ui.fixBtn.addEventListener("click", async () => {
    ui.addBtn.disabled = true;
    ui.fixBtn.disabled = true;
    ui.checkboxReplace.disabled = true;
    ui.tableContainer.disabled = true;

    ui.fixBtn.classList.remove("app-btn-primary");
    const buttonInnerHTMLBackup = ui.fixBtn.innerHTML;
    ui.fixBtn.innerHTML = '<div class="loader" style="border: 3px solid #60CDFF;"></div>';

    let results = await window.fix_banners();
    console.log(results);

    if (results.files[0] != "NO_DIR_SELECTED") {
        buildTable(results);
    }


    ui.fixBtn.innerHTML = buttonInnerHTMLBackup;
    ui.fixBtn.classList.add("app-btn-primary");

    ui.addBtn.disabled = false;
    ui.fixBtn.disabled = false;
    ui.checkboxReplace.disabled = false;


})

ui.checkboxReplace.addEventListener("click", async () => {
    await window.set_replace_files(ui.checkboxReplace.checked);
})

async function removeCia(idx) {
    let values = await window.remove_cia(idx);
    console.log(values);
    buildTable(values);
}

function buildTable(resultObj) {
    let showResults = resultObj.showResults;
    let files = resultObj.files;
    let results = resultObj.results;
    let messages = resultObj.messages;

    if (files.length > 0) {
        ui.filesTableBody.innerHTML = "";
        for (let i = 0; i < files.length; i++) {
            const tr = ui.filesTableBody.insertRow();
            const td1 = tr.insertCell();
            td1.innerHTML = files[i];
            const td2 = tr.insertCell();
            td2.innerHTML = `<button class="app-btn app-btn-subtle" onclick="removeCia(${i})"><i class="icons10-cross"></i></button>`
            if (showResults) {
                if (results[i] === true) {
                    td2.innerHTML = `<button class="app-btn app-btn-outline-success"><i class="icons10-checkmark"></i></button>` + td2.innerHTML;
                } else {
                    td2.innerHTML = `<button class="app-btn app-btn-outline-danger"><i class="icons10-exclamation-mark"></i></button>` + td2.innerHTML;
                }
            }
            td2.style.textAlign = "right";
        }
        ui.tableContainer.style.removeProperty("display");
        ui.hr1.style.removeProperty("display");
        ui.checkboxDiv.style.setProperty("display", "flex");
        ui.fixBtn.disabled = false;
        ui.fixBtn.classList.add("app-btn-primary");
    } else {
        ui.tableContainer.style.setProperty("display", "none");
        ui.checkboxDiv.style.setProperty("display", "none");
        ui.hr1.style.setProperty("display", "none");
        ui.addBtn.classList.add("app-btn-primary");
        ui.fixBtn.classList.remove("app-btn-primary");
        ui.fixBtn.disabled = true;
    }

}

function buildResultTable(resultObj) {
    ui.filesTableBody.innerHTML = "";
    let files = resultObj.files;
    let results = resultObj.results;
    let messages = resultObj.messages;

    for (let i = 0; i < files.length; i++) {
        const tr = ui.filesTableBody.insertRow();
        const td1 = tr.insertCell();
        td1.innerHTML = files[i];
        const td2 = tr.insertCell();
        if (results[i] === true) {
            td2.innerHTML = `<button class="app-btn app-btn-outline-success"><i class="icons10-checkmark"></i></button><button class="app-btn app-btn-subtle" onclick="removeCia(${i})"><i class="icons10-cross"></i></button>`
        } else {
            td2.innerHTML = `<button class="app-btn app-btn-outline-danger"><i class="icons10-exclamation-mark"></i></button><button class="app-btn app-btn-subtle" onclick="removeCia(${i})"><i class="icons10-cross"></i></button>`
        }
        td2.style.textAlign = "right";
    }
}