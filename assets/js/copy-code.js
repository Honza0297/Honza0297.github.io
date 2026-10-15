function copyText(text) {
  if (navigator.clipboard && window.isSecureContext) {
    return navigator.clipboard.writeText(text).catch(function () {
      return copyWithTextarea(text);
    });
  }
  return copyWithTextarea(text);
}

function copyWithTextarea(text) {
  return new Promise(function (resolve, reject) {
    var area = document.createElement("textarea");
    area.value = text;
    area.setAttribute("readonly", "");
    area.style.position = "fixed";
    area.style.left = "-9999px";
    document.body.appendChild(area);
    area.select();
    try {
      document.execCommand("copy") ? resolve() : reject();
    } catch (err) {
      reject(err);
    } finally {
      document.body.removeChild(area);
    }
  });
}

document.addEventListener("DOMContentLoaded", function () {
  document.querySelectorAll("div.highlighter-rouge").forEach(function (block) {
    var pre = block.querySelector("pre");
    if (!pre) return;

    var button = document.createElement("button");
    button.type = "button";
    button.className = "copy-snippet";
    button.textContent = "Copy";

    button.addEventListener("click", function () {
      var text = pre.innerText.replace(/\n$/, "");
      copyText(text).then(function () {
        button.textContent = "Copied";
        button.classList.add("copied");
        setTimeout(function () {
          button.textContent = "Copy";
          button.classList.remove("copied");
        }, 1500);
      }).catch(function () {});
    });

    block.appendChild(button);
  });
});
