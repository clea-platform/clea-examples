import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { AppComponent } from './app.component';
import { MyWebAppComponent } from './my-web-app/my-web-app.component';

@NgModule({
  declarations: [
    AppComponent,
    MyWebAppComponent
  ],
  imports: [
    BrowserModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
