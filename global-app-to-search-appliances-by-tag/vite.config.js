import { defineConfig } from "vite";

export default defineConfig(() => {
  return {
    define: {
      "process.env.NODE_ENV": `"${process.env.NODE_ENV}"`,
    },
    build: {
      lib: {
        entry: "src/index.tsx",
        formats: ["es"],
        fileName: "app",
      },
    },
    preview: {
      port: 5000,
      open: "app.mjs",
    },
  };
});
