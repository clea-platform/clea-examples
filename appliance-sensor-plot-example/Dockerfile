FROM node:20.3.1-slim as builder

WORKDIR /app
ADD package*.json ./
RUN npm ci
ADD . .
RUN npm run build

FROM nginx:1
COPY --from=builder /app/dist/app.mjs /usr/share/nginx/html/App.js
ADD nginx.conf /etc/nginx/conf.d/default.conf
