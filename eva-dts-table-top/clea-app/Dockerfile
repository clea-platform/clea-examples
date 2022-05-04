FROM node:14 as builder

WORKDIR /app
ADD . .

RUN cd data_app && \
    npm ci && \
    npm run build-bundle

FROM nginx:1
COPY --from=builder /app/data_app/dist/main.js /usr/share/nginx/html/data_app/main.js
ADD nginx.conf /etc/nginx/conf.d/default.conf
