# Clea App Angular Template

## Prerequisites

- node v14 or greater
- npm
- Angular CLI

## Develop your app

From the `angular-component` directory, install project dependencies with
```
npm install
```
You can now serve a development server with
```
ng serve --open
```
This watches the source code for changes and rebuilds the component when needed

## Build your app

From the `angular-component` directory, install project dependencies (if you haven't already) with
```
npm install
```
and then build the angular component
```
ng build
```
This will produce 2 `.js` files inside the `dist/angular-component/` directory. `main.########.js` and `polyfills.########.js`.
Copy those files inside the `app-wrapper/src/angular/` directory and strip the hash from the name.

From `app-wrapper` you can now build the app bundle with `npm run build`