const getElements = ids => Object.assign({}, ...ids.map(id => ({ [id]: document.getElementById(id) })));
const ui = getElements(["closeButton", "minButton", "maxButton", "windowBarCenter"]);

ui.closeButton.addEventListener("click", async () => {
    saucer.exposed.quit();
});

ui.maxButton.addEventListener("click", async () => {
    saucer.exposed.maximize();
});

ui.minButton.addEventListener("click", async () => {
    saucer.exposed.minimize();
});