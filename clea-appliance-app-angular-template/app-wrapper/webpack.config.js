const path = require("path");

module.exports = {
  mode: "production",
  entry: "./src/index.tsx",
  experiments: {
    outputModule: true,
  },
  output: {
    filename: "main.js",
    path: path.resolve(__dirname, "dist"),
    library: {
      type: "module",
    },
  },
  resolve: {
    extensions: [".tsx", ".ts", ".js"],
  },
  module: {
    rules: [
      {
        test: /angular\/.*\.js$/,
        type: "asset/source",
      },
      {
        test: /\.css$/i,
        use: "css-loader",
      },
      {
        test: /\.tsx?$/,
        use: "ts-loader",
        exclude: /node_modules/,
      },
    ],
  },
};
