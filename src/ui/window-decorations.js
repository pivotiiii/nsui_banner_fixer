const getElementsDeco = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const uiDeco = getElementsDeco(["closeButton", "minButton", "maxButton", "windowBarCenter", "colorButton", "maxIcon", "unmaxIcon"]);
let maximized = false;

uiDeco.closeButton.addEventListener("click", async () => {
    saucer.exposed.quit();
});

uiDeco.maxButton.addEventListener("click", async () => {
    if (maximized) {
        saucer.exposed.unmaximize();
        maximized = false;
        uiDeco.unmaxIcon.classList.add("hide-icon");
        uiDeco.maxIcon.classList.remove("hide-icon");
    } else {
        saucer.exposed.maximize();
        maximized = true;
        uiDeco.maxIcon.classList.add("hide-icon");
        uiDeco.unmaxIcon.classList.remove("hide-icon");
    }
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