# Coffee Machine Retrofitting

This repository contains the POC for the Clea Coffee Machine Retrofitting project.

## Quickstart

Enter inside the directory for a generic app. E.g.:

```
cd data_app
```

Install all the dependencies and build the required bundle.

```
npm ci
npm run build-bundle
```

This will generate inside the directory `dist/` a single ES Module bundle called `main.js` that can be used by the Clea Platform to mount and unmount the whole application.

It can be tested running a local live server from the `index.html` in the root directory (e.g. using the VSCode extension `Live Server`) that simulates the behavior of the platform.

## Configuration

Each app configuration can be tuned inside `webpack.config.js` that handles the whole build process.

### Compiles and hot-reloads for development

```
npm run dev
```
