import { ComponentFixture, TestBed } from '@angular/core/testing';

import { MyWebAppComponent } from './my-web-app.component';

describe('MyWebAppComponent', () => {
  let component: MyWebAppComponent;
  let fixture: ComponentFixture<MyWebAppComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ MyWebAppComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(MyWebAppComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
