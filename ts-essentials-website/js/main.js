// T.S Essentials: small enhancements only. The site works without this file.
(function () {
  document.documentElement.classList.add("js");

  // Mobile menu
  var toggle = document.querySelector(".nav-toggle");
  var nav = document.getElementById("site-nav");
  if (toggle && nav) {
    toggle.addEventListener("click", function () {
      var open = nav.classList.toggle("is-open");
      toggle.setAttribute("aria-expanded", open ? "true" : "false");
      toggle.textContent = open ? "Close" : "Menu";
    });
    document.addEventListener("keydown", function (e) {
      if (e.key === "Escape" && nav.classList.contains("is-open")) {
        nav.classList.remove("is-open");
        toggle.setAttribute("aria-expanded", "false");
        toggle.textContent = "Menu";
        toggle.focus();
      }
    });
  }

  // Enquiry form: builds an email in the visitor's own mail app.
  var form = document.getElementById("enquiry-form");
  if (!form) return;
  var status = document.getElementById("form-status");

  function value(id) {
    var el = document.getElementById(id);
    return el ? el.value.trim() : "";
  }

  form.addEventListener("submit", function (e) {
    e.preventDefault();
    if (!form.reportValidity()) return;

    var to = form.getAttribute("data-email") || "";
    var horse = value("horse-name");
    var subject = "Session request" + (horse ? " for " + horse : "");
    var lines = [
      "Name: " + value("owner-name"),
      "Email: " + value("owner-email"),
      "Phone: " + value("owner-phone"),
      "Barn location: " + value("location"),
      "",
      "Horse's name: " + horse,
      "Age: " + value("horse-age"),
      "Breed: " + value("horse-breed"),
      "Discipline: " + value("horse-work"),
      "Veterinarian: " + value("vet"),
      "",
      "What I'd like help with:",
      value("message")
    ];

    var href = "mailto:" + to +
      "?subject=" + encodeURIComponent(subject) +
      "&body=" + encodeURIComponent(lines.join("\n"));

    status.hidden = false;
    status.textContent = "Your email app should now open with your details filled in. " +
      "Press send in your email app to submit your request. If nothing opens, email me directly at " +
      to + ".";
    window.location.href = href;
  });
})();
