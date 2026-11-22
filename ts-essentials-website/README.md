# T.S Essentials website

A five-page static website for T.S Essentials, an equine sports massage and bodywork business. It's plain HTML, CSS and a little JavaScript, so it runs on any web host with no build step.

## Pages

| File | Page |
| --- | --- |
| `index.html` | Home |
| `approach.html` | The Approach: positional release, modality stacking, aromatherapy |
| `sessions.html` | Sessions: what to expect, before and after, FAQ |
| `about.html` | About you |
| `contact.html` | Book a Session (enquiry form) |

## Before you publish: fill in your details

These placeholders appear in square brackets. Use find-and-replace across all five `.html` files:

| Placeholder | Replace with |
| --- | --- |
| `[Your Name]` | Your name (About page) |
| `[Your Area]` | The area you cover, e.g. a county or region |
| `[Phone Number]` | Your phone number. Also replace it inside `href="tel:[Phone Number]"`, written without spaces (e.g. `tel:+353871234567`) |
| `[Email Address]` | Your email address. It's also used by the enquiry form |

On `about.html`, also replace:

- The paragraph that starts `[Add a few lines about your own background...`
- The three bracketed lines under **Training & qualifications**

On `approach.html`, check the **Techniques I may draw on** list and change it to match the techniques you actually use.

### Recommended

- Add a photo of yourself at work on the About page. Save it in `images/` and change the `src` of the first image in `about.html`.
- Over time, swap the stock photos for photos of horses you've worked on (with their owners' permission).

## The enquiry form

The form doesn't need a server. When someone presses **Continue to email**, it opens their email app with their answers filled in, addressed to the email you set above. They then press send.

If you'd prefer enquiries to arrive without the visitor using their own email app, you can connect the form to a form service (for example Formspree or Netlify Forms) later.

## Hosting

Upload the whole `ts-essentials-website` folder to any static host, such as Netlify, GitHub Pages, Cloudflare Pages, or your domain provider's hosting. `index.html` is the home page.

## Images

- `emblem*.webp`, `favicon.png`, `apple-touch-icon.png` and `logo-full.jpg` are made from your logo.
- `knot-band.webp` is the Celtic knot ring from your logo, unrolled into a straight band. It's used as the decorative divider throughout the site.
- All horse photographs are released under CC0 (public domain dedication) via Unsplash and Wikimedia Commons, so no credit is required. Credits for reference:
  - `farm-sunset` by Omar Prestwich
  - `chestnut-blaze` by Gert Boers
  - `horse-eye` by Jez Timms
  - `horse-grazing` by Annie Spratt
  - `bay-halter` by McCall Alexander
  - `horses-nuzzling` by Thibault Carron
  - `horses-cold-morning` by Anders Jildén
  - `palomino-fog` by Henry Chuy
  - `handler-palomino` by Jennifer Murray

## Fonts

Cinzel (brand and labels, chosen to match the lettering in the logo), Alegreya (headings) and Alegreya Sans (body text), loaded from Google Fonts.
