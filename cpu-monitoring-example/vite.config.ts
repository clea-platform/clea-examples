import { defineConfig, LibraryFormats } from "vite";

export default defineConfig(() => {
  return {
    define: {
      "process.env.NODE_ENV": `"${process.env.NODE_ENV}"`,
    },
    build: {
      lib: {
        entry: "src/index.tsx",
        formats: ["es"] as LibraryFormats[],
        fileName: "app",
      },
    },
    preview: {
      port: 5000,
      open: "app.mjs",
    },
  };
});
