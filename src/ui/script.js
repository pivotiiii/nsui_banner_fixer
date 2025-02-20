const getElements = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const ui = getElements([
    "filesTableBody", "addBtn", "fixBtn", "tableContainer", "hr1", "hr2", "checkboxDiv", "checkboxReplace",
    "dangerBar", "successBar", "alertCloseButtonSuccess", "alertCloseButtonError",
    "missingReqsAlert", "missingReqsAlertCloseBtn", "missingReqsAlertText",
    "infoAlert", "infoAlertCloseBtn", "infoAlertText", "infoButtonContainer"]);

ui.addBtn.addEventListener("click", async () => {
    ui.addBtn.classList.remove("app-btn-primary");
    const buttonInnerHTMLBackup = ui.addBtn.innerHTML;
    ui.addBtn.innerHTML = '<div class="loader" style="border: 3px solid #60CDFF;"></div>';
    let valuesString = await saucer.exposed.add_cias();
    let values = await JSON.parse(valuesString);
    buildTable(values);
    ui.addBtn.innerHTML = buttonInnerHTMLBackup;
})

ui.fixBtn.addEventListener("click", async () => {
    ui.addBtn.disabled = true;
    ui.fixBtn.disabled = true;
    ui.checkboxReplace.disabled = true;
    ui.tableContainer.disabled = true;

    ui.successBar.classList.remove("app-alert-bar-active");
    ui.dangerBar.classList.remove("app-alert-bar-active");

    ui.fixBtn.classList.remove("app-btn-primary");
    const buttonInnerHTMLBackup = ui.fixBtn.innerHTML;
    ui.fixBtn.innerHTML = '<div class="loader" style="border: 3px solid #60CDFF;"></div>';

    let resultsString = await saucer.exposed.fix_banners();
    let results = JSON.parse(resultsString);

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
    await saucer.exposed.set_replace_files(ui.checkboxReplace.checked);
})

ui.alertCloseButtonSuccess.addEventListener("click", async () => {
    ui.successBar.classList.remove("app-alert-bar-active");
})

ui.alertCloseButtonError.addEventListener("click", async () => {
    ui.dangerBar.classList.remove("app-alert-bar-active");
})

ui.missingReqsAlertCloseBtn.addEventListener("click", async () => {
    ui.missingReqsAlert.classList.remove("show");
})

window.addEventListener("load", async () => {
    await checkRequirements();
});

ui.infoAlertCloseBtn.addEventListener("click", async () => {
    ui.infoAlert.classList.remove("show");
})

ui.infoButtonContainer.addEventListener("click", async () => {
    ui.infoAlert.classList.add("show");
})

window.addEventListener("load", async () => {
    let program_info_json = await saucer.exposed.get_program_info();
    let program_info = await JSON.parse(program_info_json);
    console.log(program_info);
    let license_html = program_info.license.split("<br>").slice(2).join("<br>");
    ui.infoAlertText.innerHTML =
        "nsui_banner_fixer v" + program_info.version + "<br>"
        + "Copyright (c) " + program_info.year + " pivotiii<br>"
        + license_html + "<br><br>"
        + "compiled at " + program_info.compile_time;
});

async function removeCia(idx) {
    let valuesString = await saucer.exposed.remove_cia(idx);
    let values = await JSON.parse(valuesString);
    buildTable(values);
}

function buildTable(resultObj) {
    let showResults = resultObj.showResults;
    let files = resultObj.files;
    let results = resultObj.results;
    let messages = resultObj.messages;

    console.log(resultObj);

    if (files.length > 0) {
        ui.filesTableBody.innerHTML = "";
        let numErrors = 0;
        for (let i = 0; i < files.length; i++) {
            const tr = ui.filesTableBody.insertRow();
            const td1 = tr.insertCell();
            td1.innerHTML = files[i];
            const td2 = tr.insertCell();
            td2.innerHTML = `<button class="app-btn app-btn-subtle" onclick="removeCia(${i})"><i class="icons10-cross"></i></button>`
            if (showResults) {
                if (results[i] === true) {
                    td2.innerHTML =
                        `<button class="app-btn app-btn-outline-success" `
                        + `style="pointer-events: none; border: none;">`
                        + `<i class="icons10-checkmark"></i>`
                        + `</button>`
                        + td2.innerHTML;
                } else {
                    td2.innerHTML =
                        `<button class="app-btn app-btn-outline-danger" `
                        // + `style="pointer-events: none;">`
                        + `onclick="showFileError('${messages[i]}')">`
                        + `Error<i class="icons10-exclamation-mark"></i></button>`
                        + td2.innerHTML;
                    numErrors = numErrors + 1;
                }
            }
            td2.style.textAlign = "right";
        }
        ui.tableContainer.style.removeProperty("display");
        ui.hr1.style.removeProperty("display");
        ui.checkboxDiv.style.setProperty("display", "flex");
        ui.fixBtn.disabled = false;
        ui.fixBtn.classList.add("app-btn-primary");
        if (showResults) {
            showAlert(numErrors)
        }
    } else {
        ui.tableContainer.style.setProperty("display", "none");
        ui.checkboxDiv.style.setProperty("display", "none");
        ui.hr1.style.setProperty("display", "none");
        ui.addBtn.classList.add("app-btn-primary");
        ui.fixBtn.classList.remove("app-btn-primary");
        ui.fixBtn.disabled = true;
    }

}

function showFileError(message) {
    message = message.replaceAll("ERROR: ", "");
    ui.missingReqsAlertText.innerHTML = message;
    ui.missingReqsAlert.classList.add("show");
}

function showAlert(numErrors) {
    if (numErrors === 0) {
        ui.successBar.classList.add("app-alert-bar-active");
        setTimeout(() => {
            ui.successBar.classList.remove("app-alert-bar-active");
        }, 3500);
    } else {
        ui.dangerBar.classList.add("app-alert-bar-active");
        setTimeout(() => {
            ui.dangerBar.classList.remove("app-alert-bar-active");
        }, 3500);
        if (numErrors === 1) {
            ui.dangerBar.children[2].innerHTML = "There has been " + numErrors + " error.";
        } else {
            ui.dangerBar.children[2].innerHTML = "There have been " + numErrors + " errors.";
        }

    }
}

async function checkRequirements(repeat = false) {
    let resultsString = await saucer.exposed.check_requirements();
    let results = JSON.parse(resultsString);
    console.log(results);
    if (repeat === true && results.result === false) {
        setTimeout(() => {
            checkRequirements(true);
        }, 3000);
    }
    else if (repeat === true && results.result === true) {
        ui.addBtn.disabled = false;
        ui.addBtn.classList.add("app-btn-primary");
    }
    else if (results.result === false) {
        let missing_files_string = results.missing_files.join("<br>");
        ui.missingReqsAlertText.innerHTML =
            "Missing required files!<br><br class=\"smaller\">"
            + missing_files_string
            + "<br><br class=\"smaller\">"
            + "Please make sure you are running nsui_banner_fixer.exe in a directory in which you have write access. (e.g. outside of Program Files)";
        ui.addBtn.disabled = true;
        ui.addBtn.classList.remove("app-btn-primary");
        ui.missingReqsAlert.classList.add("show");
        setTimeout(() => {
            checkRequirements(true);
        }, 3000);
    }
}