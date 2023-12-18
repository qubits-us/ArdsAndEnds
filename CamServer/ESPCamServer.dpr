program ESPCamServer;

uses
  Vcl.Forms,
  frmCamView in 'frmCamView.pas' {CamViewFrm},
  Vcl.Themes,
  Vcl.Styles,
  frmCamServer in 'frmCamServer.pas' {CamServerFrm},
  frmReplay in 'frmReplay.pas' {ReplayFrm};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  TStyleManager.TrySetStyle('Windows11 Modern Dark');
  Application.CreateForm(TCamServerFrm, CamServerFrm);
  Application.Run;
end.
