FROM node:22.13.1-slim AS builder

WORKDIR /app
ADD . .

RUN cd cpu-monitoring-example && \
    npm ci && \
    npm run build

FROM nginx:1
COPY --from=builder /app/cpu-monitoring-example/dist/app.js /usr/share/nginx/html/cpu-monitoring-example/main.js
ADD nginx.conf /etc/nginx/conf.d/default.conf
