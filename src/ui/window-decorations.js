const getElementsDeco = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const uiDeco = getElementsDeco(["closeButton", "minButton", "maxButton", "windowBarCenter", "colorButton", "maxIcon", "unmaxIcon"]);
const r = document.querySelector(':root');
const resizeEdges = document.querySelectorAll(".hide-when-maximized");
const dragMaxEdges = document.querySelectorAll(".drag-when-maximized");

uiDeco.closeButton.addEventListener("click", async () => {
    saucer.exposed.quit();
});

uiDeco.maxButton.addEventListener("click", async () => {
    saucer.exposed.toggle_maximize();
});

uiDeco.minButton.addEventListener("click", async () => {
    saucer.exposed.minimize();
});

uiDeco.colorButton.addEventListener("click", async () => {
    if (Appearance.getColorScheme() === "dark") {
        Appearance.setLightScheme()
    } else {
        Appearance.setDarkScheme()
    }
});

function setMaxDeco(maxState) {
    if (maxState === true) {
        uiDeco.maxIcon.classList.add("hide-icon");
        uiDeco.unmaxIcon.classList.remove("hide-icon");
        uiDeco.closeButton.style.setProperty("margin-left", "0px");
        for (let index = 0; index < resizeEdges.length; index++) {
            resizeEdges[index].classList.add("hide-icon");
        }
        for (let index = 0; index < dragMaxEdges.length; index++) {
            dragMaxEdges[index].setAttribute("data-webview-drag", "");
        }
        r.style.setProperty("--border-color", "var(--dark, rgb(17, 17, 17)) var(--light, rgb(255, 255, 255))");
    } else {
        uiDeco.unmaxIcon.classList.add("hide-icon");
        uiDeco.maxIcon.classList.remove("hide-icon");
        uiDeco.closeButton.style.setProperty("margin-left", "3px");
        for (let index = 0; index < resizeEdges.length; index++) {
            resizeEdges[index].classList.remove("hide-icon");
        }
        for (let index = 0; index < dragMaxEdges.length; index++) {
            dragMaxEdges[index].removeAttribute("data-webview-drag");
        }
        r.style.setProperty("--border-color", "var(--dark, #333333) var(--light, #8a8a8a)");
    }
}