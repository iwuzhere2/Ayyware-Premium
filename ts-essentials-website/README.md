# T.S Essentials website

A five-page static website for T.S Essentials, an equine sports massage and bodywork business. It's plain HTML, CSS and a little JavaScript, so it runs on any web host with no build step.

## Pages

| File | Page |
| --- | --- |
| `index.html` | Home |
| `services.html` | Services: positional release, sports massage, structural alignment, modality stacking, aromatherapy |
| `sessions.html` | Sessions: what to expect, preparation, aftercare, FAQ |
| `about.html` | About: six years in the industry, training under Jeffra Sinclair |
| `contact.html` | Book a Session (request form) |

## Before you publish: fill in your details

Two placeholders remain. Use find-and-replace across all five `.html` files:

| Placeholder | Replace with |
| --- | --- |
| `[Phone Number]` | Your phone number. Inside `href="tel:[Phone Number]"`, write it without spaces or dashes (e.g. `tel:+13525550123`) |
| `[Email Address]` | Your email address. The request form also sends to it |

### Recommended

- Add a photo of yourself at work on the About page. Save it in `images/` and change the `src` of the first image in `about.html`.
- If you hold specific certifications or liability insurance, add them to the "At a glance" list on the About page.
- Over time, swap the stock photos for photos of horses you've worked on (with their owners' permission).

### A note on wording

The site describes your work as "structural alignment", "bodywork", and "sessions". It never describes it as chiropractic, adjustments, diagnosis, or treatment; those words appear only in the disclaimers that say what the work is not. The Florida Veterinary Medical Association's position is that animal chiropractic falls outside a non-veterinarian's authority, and diagnosing or treating animal injury is veterinary practice under Chapter 474, Florida Statutes. Keep that wording if you edit the copy. This isn't legal advice; for certainty, check with a Florida veterinary attorney or the Florida Board of Veterinary Medicine.

## The request form

The form doesn't need a server. When someone presses **Continue to email**, it opens their email app with their answers filled in, addressed to the email you set above. They then press send.

If you'd prefer requests to arrive without the visitor using their own email app, you can connect the form to a form service (for example Formspree or Netlify Forms) later.

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
