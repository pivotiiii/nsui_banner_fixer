const getElementsDeco = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const uiDeco = getElementsDeco(["closeButton", "minButton", "maxButton", "windowBarCenter", "colorButton"]);

uiDeco.closeButton.addEventListener("click", async () => {
    saucer.exposed.quit();
});

uiDeco.maxButton.addEventListener("click", async () => {
    saucer.exposed.maximize();
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