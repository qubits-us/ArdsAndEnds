program AckMe;

uses
  Vcl.Forms,
  frmMain in 'frmMain.pas' {MainFrm},
  Vcl.Themes,
  Vcl.Styles;

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  TStyleManager.TrySetStyle('Windows11 MineShaft');
  Application.CreateForm(TMainFrm, MainFrm);
  Application.Run;
end.
