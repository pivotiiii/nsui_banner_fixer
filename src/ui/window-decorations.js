const getElementsDeco = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const uiDeco = getElementsDeco(["closeButton", "minButton", "maxButton", "windowBarCenter", "colorButton"]);
const r = document.querySelector(":root");

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
        r.style.setProperty("--window-bar-background-color", "rgb(255, 255, 255)");
        uiDeco.colorButton.children[0].classList.remove("icons10-sun");
        uiDeco.colorButton.children[0].classList.add("icons10-moon");
    } else {
        Appearance.setDarkScheme()
        r.style.setProperty("--window-bar-background-color", "rgb(17, 17, 17)");
        uiDeco.colorButton.children[0].classList.remove("icons10-moon");
        uiDeco.colorButton.children[0].classList.add("icons10-sun");
    }
});

if (Appearance.getColorScheme() === "dark") {
    r.style.setProperty("--window-bar-background-color", "rgb(17, 17, 17)");
    uiDeco.colorButton.children[0].classList.remove("icons10-moon");
    uiDeco.colorButton.children[0].classList.add("icons10-sun");
} else {
    r.style.setProperty("--window-bar-background-color", "rgb(255, 255, 255)");
    uiDeco.colorButton.children[0].classList.remove("icons10-sun");
    uiDeco.colorButton.children[0].classList.add("icons10-moon");
}