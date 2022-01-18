FROM node:14 as builder

WORKDIR /app
ADD package*.json ./
RUN npm ci
ADD . .
RUN npm run build

FROM nginx:1
COPY --from=builder /app/dist/main.js /usr/share/nginx/html/PeopleCountingApplianceApp.js
ADD nginx.conf /etc/nginx/conf.d/default.conf