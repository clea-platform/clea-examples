# Global App to search appliances by tag

This example shows how a Clea App with a Global scope can perform GraphQL queries to list existing appliances, optionally filtering them by tag.

## Technologies

The example is implemented with the following technologies:

- [Vite](https://vitejs.dev/) for bundling and building the app.
- [Apollo](https://www.apollographql.com/docs/react/) to perform GraphQL queries.
- [react-bootstrap](https://react-bootstrap.github.io/) for react-ready Bootstrap components.
- [react-intl](https://formatjs.io/docs/react-intl/) for internationalization support.

## How to use

Install [NodeJS](https://nodejs.org/) and NPM. If using the [asdf](https://asdf-vm.com/) runtime version manager, you can do that with:

```
asdf plugin-add nodejs
asdf install
```

Then run `npm run build` to bundle the app, or `npm run watch` to continuosly bundle it whenever source files change.

Serve the app bundle by running `npm run serve`.
