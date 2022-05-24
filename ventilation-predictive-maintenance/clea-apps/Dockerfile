FROM node:14 as builder

WORKDIR /app
ADD . .

RUN cd predictive-maintenance_app && \
    npm ci && \
    npm run build-bundle

FROM nginx:1
COPY --from=builder /app/predictive-maintenance_app/dist/main.js /usr/share/nginx/html/predictive-maintenance_app/main.js
ADD nginx.conf /etc/nginx/conf.d/default.conf
