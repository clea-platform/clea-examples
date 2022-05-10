# Clea Dashboard
<!-- TOC -->

- [Clea Dashboard](#clea-dashboard)
    - [Quickstart](#quickstart)
    - [Configuration](#configuration)
    - [Compiles and hot-reloads for development](#compiles-and-hot-reloads-for-development)
    - [Project Structure](#project-structure)
        - [index.tsx](#indextsx)
        - [AstarteClient.ts](#astarteclientts)
        - [utils](#utils)
            - [aggregate.ts](#aggregatets)
        - [types](#types)
        - [pages](#pages)
        - [lang](#lang)

<!-- /TOC -->

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

## Compiles and hot-reloads for development

For development purposes, you can uncomment the testing part inside `index.tsx` for each app using the classic React hot-reload feature:

```
npm run dev
```

## Project Structure

### `index.tsx`

The entrypoint of each app resides inside the `index.tsx` file. The render inject some custom hardcoded CSS plus some other dependencies like _bootstrap-icons_

Some custom settings for the datetime formatting can be defined inside this block:

```typescript
moment.updateLocale(language, {
  week: {
    dow: 1, // Monday is the first day of the week.
  },
});
```

### `AstarteClient.ts`

The `AstarteClient` module defines the procedures to fetch data from the platform based on the specified device ID etc.

### `utils`

Inside this subdirectory you can find some utility modules.

#### `aggregate.ts`

This module defines the logic to aggregate the fetched Astarte data ordering it by:

| Hour | Day | Week | Month | Year |
| ---- | --- | ---- | ----- | ---- |

### `types`

Define interfaces and types inside here to use them globally inside the project.

### `pages`

Each page inside the app tab will go here. Ideally, since we're working with SPA, you have to define just the main page here.

### `lang`

This directory contains every language word definitions. They are stored as a json file that will be flattened and injected inside the app leveraging the `IntlProvider`.