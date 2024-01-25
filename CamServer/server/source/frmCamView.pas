unit frmCamView;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, OverbyteIcsWndControl, OverbyteIcsWSocket, Vcl.StdCtrls, Vcl.ExtCtrls,VCL.Imaging.jpeg, Vcl.ExtDlgs,
  OverbyteIcsWSocketS,System.IOUtils,System.DateUtils, Vcl.WinXCtrls,
  Vcl.ComCtrls, Vcl.NumberBox,uCamServerData;




type
     TPacketHeader = packed record
       PackIdent :array[0..1] of byte;
       Command   :byte;
       DataSize  :UINT32;
     end;

type
   TCamSync = packed record
     Year :word;
     month:byte;
     day  :byte;
     hour :byte;
     min  :byte;
     sec  :byte;
   end;





const
    MAX_IMSIZE =500000;
    IDENT_LO   =113;
    IDENT_HI   =126;

    CMD_NAK        =0;
    CMD_ACK        =1;
    CMD_SYNC       =2;
    CMD_IDENT      =3;
    CMD_JPG_BEGIN  =4;
    CMD_JPG_CHUNK  =5;
    CMD_JPG_END    =6;
    CMD_PAUSE      =7;
    CMD_CONFIG_GET =8;
    CMD_CONFIG_SET =9;
    CMD_REC_ON     =10;
    CMD_REC_OFF    =11;
    CMD_ALERT_ON   =12;
    CMD_ALERT_OFF  =13;
    CMD_OTA_BEGIN  =14;
    CMD_OTA_CHUNK  =15;
    CMD_OTA_END    =16;
    CMD_STATE_GET  =17;

    JPG_CHUNK_SIZE  =1436;
    OTA_CHUNK_SIZE  =4096;



type
  TCamViewFrm = class(TForm)
    pnlTop: TPanel;
    pnlBottom: TPanel;
    imgCam: TImage;
    btnConfig: TButton;
    sckCam: TWSocket;
    mLog: TMemo;
    edBadPacks: TEdit;
    btnSave: TButton;
    dlgSavePic: TSavePictureDialog;
    chkDebug: TCheckBox;
    chkRecord: TCheckBox;
    tmrTimedOut: TTimer;
    pnlConfig: TPanel;
    cmbRes: TComboBox;
    tbQuality: TTrackBar;
    tbBrightness: TTrackBar;
    tbContrast: TTrackBar;
    cmbEffect: TComboBox;
    tsAWB: TToggleSwitch;
    tsAWBgain: TToggleSwitch;
    cmbWBmode: TComboBox;
    tsAECsensor: TToggleSwitch;
    tsAECDSP: TToggleSwitch;
    tbAElevel: TTrackBar;
    tsAGC: TToggleSwitch;
    tbGainCeiling: TTrackBar;
    tsColorBar: TToggleSwitch;
    tsDCW: TToggleSwitch;
    tsVFlip: TToggleSwitch;
    tsHmirror: TToggleSwitch;
    tsLensCorr: TToggleSwitch;
    tsRawGMA: TToggleSwitch;
    tsWPC: TToggleSwitch;
    tsBPC: TToggleSwitch;
    lblRes: TLabel;
    lblQuality: TLabel;
    lblBrightness: TLabel;
    lblContrast: TLabel;
    lblSaturation: TLabel;
    lblEffect: TLabel;
    lblAWB: TLabel;
    lblAWBgain: TLabel;
    lblWBmode: TLabel;
    lblAecSensor: TLabel;
    lblAecDsp: TLabel;
    lblAElevel: TLabel;
    lblAGC: TLabel;
    lblBPC: TLabel;
    lblWpc: TLabel;
    lblRawGMA: TLabel;
    lblLensCorr: TLabel;
    lblHmirror: TLabel;
    lblVFlip: TLabel;
    lblDCW: TLabel;
    lblColorBar: TLabel;
    lblGainCeiling: TLabel;
    tbSaturation: TTrackBar;
    tbExposure: TTrackBar;
    lblExposure: TLabel;
    lblGain: TLabel;
    tbGain: TTrackBar;
    btnGetConfig: TButton;
    btnPutConfig: TButton;
    lblRecvInfo: TLabel;
    lblRecordInfo: TLabel;
    tmrAlert: TTimer;
    cbSilenceAlert: TCheckBox;
    cbRemoteRecord: TCheckBox;
    GroupBox1: TGroupBox;
    cmbRecordMode: TComboBox;
    lblRecMode: TLabel;
    edRecInterval: TEdit;
    lblRecInterval: TLabel;
    btnRecApply: TButton;
    tmrRecord: TTimer;
    btnPreview: TButton;
    tsDebug: TToggleSwitch;
    lblDebug: TLabel;
    edCapDelay: TEdit;
    lblCapDelay: TLabel;
    lblRejected: TLabel;
    procedure sckCamDataAvailable(Sender: TObject; ErrCode: Word);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure sckCamSessionConnected(Sender: TObject; ErrCode: Word);
    procedure sckCamSessionClosed(Sender: TObject; ErrCode: Word);
    procedure sckCamError(Sender: TObject);
    procedure sckCamException(Sender: TObject; SocExcept: ESocketException);
    procedure sckCamSocksConnected(Sender: TObject; ErrCode: Word);
    procedure sckCamSocksError(Sender: TObject; Error: Integer; Msg: string);
    procedure sckCamBgException(Sender: TObject; E: Exception; var CanClose: Boolean);
    procedure btnConfigClick(Sender: TObject);
    procedure btnSaveClick(Sender: TObject);
    procedure chkDebugClick(Sender: TObject);
    procedure chkRecordClick(Sender: TObject);
    procedure StartRecording;
    procedure StopRecording;
    procedure SaveFrame;
    procedure piProcessIncoming;
    procedure piProcessAck;
    procedure piProcessNak;
    procedure piProcessIdent;
    procedure piProcessSync;
    procedure piProcessJpgBegin;
    procedure piProcessJpgChunk;
    procedure piProcessJpgEnd;
    procedure piProcessOtaChunk;
    procedure piProcessRecOn;
    procedure piProcessRecOff;
    procedure piProcessAlertOn;
    procedure piProcessAlertOff;
    procedure piProcessConfigGet;
    procedure piProcessConfigSet;
    procedure piSkip;
    procedure tmrTimedOutTimer(Sender: TObject);
    procedure tsAGCClick(Sender: TObject);
    procedure tsAECsensorClick(Sender: TObject);
    procedure btnGetConfigClick(Sender: TObject);
    procedure imgCamClick(Sender: TObject);
    procedure tmrAlertTimer(Sender: TObject);
    procedure cbSilenceAlertClick(Sender: TObject);
    procedure btnRecApplyClick(Sender: TObject);
    procedure btnPutConfigClick(Sender: TObject);
    procedure LoadCamSettings;
    procedure LoadCamSensor;
    procedure SaveCamSensor;
    procedure tmrRecordTimer(Sender: TObject);
    procedure btnPreviewClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  CamNum:integer;
  inBuff:TBytes;
  imgBuff :TBytes;
  imgSize:Int32;
  imgRecvd:integer;
  RecvCount:integer;
  RecvState:byte;
  haveRecvdImg:boolean;
  badPacks:integer;
  CamStrm:tMemoryStream;
  NumImgs:integer;
  haveImage:boolean;
  debugging:boolean;
  //recording vars..
  strmFile:TBufferedFileStream;
  vFolder:String;
  vFilePath:String;
  vFileName:String;
  recording:boolean;
  canRecord:boolean;
  RecordMode:integer;
  RecordFrame:boolean;
  //our packets..
  hdr:TPacketHeader;
  ident:TCamId;
  sync:TCamSync;
  sensor:TCamSensor;

  //remember form h,w
  remHeight:integer;
  remWidth:integer;
  oh,ow:integer;
  //are we showing config..
  showingConfig:boolean;
  //our camera..
  Camera:TEspCamera;
  //allow cam to start or alert..
  AllowRemoteAlerts:boolean;
  AllowRemoteRecord:boolean;
  //what we got to work with..
  RecordSpaceFree:int64;
  RecordSpaceTotal:int64;
  end;

var
  CamViewFrm: TCamViewFrm;

implementation

{$R *.dfm}


procedure TCamViewFrm.LoadCamSensor;
begin
  //load cam sensor vars into controls..
   cmbRes.ItemIndex      := Camera.Sensor.FrameSize;
   tbQuality.Position    := Camera.Sensor.Quality;
   tbBrightness.Position := Camera.Sensor.Brightness;
   tbContrast.Position   := Camera.Sensor.Contrast;
   tbSaturation.Position := Camera.Sensor.Saturation;
   cmbEffect.ItemIndex   := Camera.Sensor.SpecialEffect;
   tsAWB.State           := TToggleSwitchState(Camera.Sensor.Awb);
   tsAWBgain.State       := TToggleSwitchState(Camera.Sensor.AwbGain);
   cmbWBmode.ItemIndex   := Camera.Sensor.WbMode;
   tsAECsensor.State     := TToggleSwitchState(Camera.Sensor.Aec);
   tsAECDSP.State        := TToggleSwitchState(Camera.Sensor.Aec2);
   tbAElevel.Position    := Camera.Sensor.AeLevel;
   tbExposure.Position   := Camera.Sensor.AecValue;
   tsAGC.State           := TToggleSwitchState(Camera.Sensor.Agc);
   tbGainCeiling.Position:= Camera.Sensor.GainCeiling;
   tbGain.Position       := Camera.Sensor.AgcGain;
   tsBPC.State           := TToggleSwitchState(Camera.Sensor.Bpc);
   tsWPC.State           := TToggleSwitchState(Camera.Sensor.Wpc);
   tsRawGMA.State        := TToggleSwitchState(Camera.Sensor.Raw_gma);
   tsLensCorr.State      := TToggleSwitchState(Camera.Sensor.Lenc);
   tsHMirror.State       := TToggleSwitchState(Camera.Sensor.HorzMirror);
   tsVFlip.State         := TToggleSwitchState(Camera.Sensor.VertFlip);
   tsDCW.State           := TToggleSwitchState(Camera.Sensor.Dcw);
   tsColorBar.State      := TToggleSwitchState(Camera.Sensor.Colorbar);
   tsDebug.State         := TToggleSwitchState(Camera.Sensor.Debug);
   edCapDelay.Text       := IntToStr(Camera.Sensor.CapInterval);
end;

procedure TCamViewFrm.SaveCamSensor;
var
s:TCamSensor;
begin
 //save vars back into cam sensor..
   s.FrameSize                      := cmbRes.ItemIndex;
   s.Quality                        := tbQuality.Position;
   s.Brightness                     := tbBrightness.Position;
   s.Contrast                       := tbContrast.Position;
   s.Saturation                     := tbSaturation.Position;
   s.SpecialEffect                  := cmbEffect.ItemIndex;
   TToggleSwitchState(s.Awb)        := tsAWB.State;
   TToggleSwitchState(s.AwbGain)    := tsAWBgain.State;
   s.WbMode                         := cmbWBmode.ItemIndex;
   TToggleSwitchState(s.Aec)        := tsAECsensor.State;
   TToggleSwitchState(s.Aec2)       := tsAECDSP.State;
   s.AeLevel                        := tbAElevel.Position;
   s.AecValue                       := tbExposure.Position;
   TToggleSwitchState(s.Agc)        := tsAGC.State;
   s.GainCeiling                    := tbGainCeiling.Position;
   s.AgcGain                        := tbGain.Position;
   TToggleSwitchState(s.Bpc)        := tsBPC.State;
   TToggleSwitchState(s.Wpc)        := tsWPC.State;
   TToggleSwitchState(s.Raw_gma)    := tsRawGMA.State;
   TToggleSwitchState(s.Lenc)       := tsLensCorr.State;
   TToggleSwitchState(s.HorzMirror) := tsHMirror.State;
   TToggleSwitchState(s.VertFlip)   := tsVFlip.State;
   TToggleSwitchState(s.Dcw)        := tsDCW.State;
   TToggleSwitchState(s.Colorbar)   := tsColorBar.State;
   TToggleSwitchState(s.Debug)      := tsDebug.State;
   s.CapInterval                    := StrToInt(edCapDelay.Text);
    //put sensor to out cam object..
   Camera.Sensor := s;
end;


procedure TCamViewFrm.LoadCamSettings;
var
aStr:String;
ndx:integer;
begin
//get the recording folder..
 aStr :=GUIDToString(Camera.CamId);
 if Length(aStr) >0 then
   begin
    //remove braces..
    ndx:=Pos('{',aStr);
    if ndx >0 then Delete(aStr,ndx,1);
    ndx:=Pos('}',aStr);
    if ndx >0 then Delete(aStr,ndx,1);

    vFilePath:=ExtractFilePath(Application.ExeName);
    vFilePath:=vFilePath+'video\'+aStr+'\';

     //create the cam root folder..
    {$I-}
     MkDir(vFilePath);
    {$I+}

    //postion and size wnd
    Left   := Camera.Left;
    Top    := Camera.Top;
    Height := Camera.Height;
    Width  := Camera.Width;
    Caption := 'ESP Cam:'+IntToStr(Camera.CamNum);
    cmbRecordMode.ItemIndex := Camera.RecMode;
    edRecInterval.Text:=IntToStr(Camera.RecIntv);

    //load the sensor
    LoadCamSensor;
   end;

end;


procedure TCamViewFrm.FormCreate(Sender: TObject);
var
ndx:integer;
begin
 //
 vFilePath:=ExtractFilePath(Application.ExeName);
 vFilePath:=vFilePath+'video\';
if  System.SysUtils.GetDiskFreeSpaceEx(PChar(vFilePath), RecordSpaceFree, RecordSpaceTotal, nil) then
 begin
   lblRecordInfo.Caption:='Record space Total: '+IntToStr(RecordSpaceTotal div (1024*1024*1024))+' GB Free: '+
   IntToStr(RecordSpaceFree div (1024*1024*1024))+' GB';
  end;
 //one folder for each day..
 vFolder:=FormatDateTime('YYYY-MM-DD', now);
 Camera:=TEspCamera.Create;
 Camera.NewRec(1);
 SetLength(inBuff,MAX_IMSIZE);
 SetLength(imgBuff,MAX_IMSIZE);
 haveRecvdImg:=false;
 imgSize:=0;
 RecvCount:=0;
 RecvState := 0;
 CamStrm:=tMemoryStream.Create;
 mLog.Lines.Insert(0,'Ready..');
 sckCam.BufSize:=MAX_IMSIZE;
 NumImgs:=0;
 BadPacks:=0;
 haveImage:=false;
 debugging:=false;
 CamNum:=0;
 recording:=false;
 showingConfig := false;
 remHeight := 0;
 AllowRemoteAlerts:=false;
 AllowRemoteRecord:=false;
 //need more than a free gig to record..
 if (RecordSpaceFree div (1024*1024*1024))>1 then
  begin
   chkRecord.Enabled := true;
   canRecord := true;
   end else canRecord := false;


end;


procedure TCamViewFrm.imgCamClick(Sender: TObject);
begin
if (not showingConfig) and (not debugging) then
  begin
    if pnlTop.Visible then
      begin
      Height := oh;
      Width :=  ow;
      pnlTop.Visible := false;
      pnlBottom.Visible := false;
      mLog.Visible := false;
      end else
        begin
          oh := Height;
          ow := Width;
          if Height < 300 then
              Height :=Height + 140;
          if Width < 440 then Width := 440;
        pnlTop.Visible := true;
        pnlBottom.Visible := true;
        mLog.Visible := true;
        end;
  end;
end;

procedure TCamViewFrm.FormClose(Sender: TObject; var Action: TCloseAction);
var
ndx:integer;
begin
tmrRecord.Enabled   := false;
tmrTimedOut.Enabled := false;

sckCam.Abort;

if recording then
 begin
   recording := false;
  if Assigned(strmFile) then
  begin
   strmFile.FlushBuffer;
   strmFile.Free;
   strmFile := nil;
  end;
 end;

 //save any changes made to config..
 SaveCamSensor;

//Save local cam to global data..
 ndx:=CamSrvrDat.FindCam(Camera.CamId);
 if ndx > -1 then
   begin
     Camera.Top := Top;
     Camera.Left := Left;
    if (not showingConfig) and (not debugging) then
     begin
     Camera.Height:=Height;
     Camera.Width:=Width;
     end else
      begin
      Camera.Height:=oh;
      Camera.Width:=ow;
      end;
     CamSrvrDat.Cam[ndx] := Camera;
   end;
 //free cam stream
 try
 sckCam.Close;
 finally
   //
 end;

 //tell server form to free us..
 try
 PostMessage(TForm(Owner).Handle, WM_USER, 0, LongInt(Self));
 finally
   //
 end;
end;

procedure TCamViewFrm.btnConfigClick(Sender: TObject);
var
h:TPacketHeader;
begin
if pnlConfig.Visible then
  begin
    pnlConfig.Visible := false;
    Height := remHeight;
    Width := remWidth;
    showingConfig := false;
    //unpause the video..
   h.PackIdent[0] := IDENT_LO;
   h.PackIdent[1] := IDENT_HI;
   h.Command:=CMD_JPG_BEGIN;
   h.DataSize :=0;
   sckCam.Send(@h,sizeof(TPacketHeader));
  end else
    begin
      pnlConfig.Visible := true;
      remHeight := Height;
      remWidth := Width;
      Width := Width + 550;
      if Height < 756 then Height := 756;
      showingConfig := true;
    end;
end;

procedure TCamViewFrm.btnGetConfigClick(Sender: TObject);
var
h:TPacketHeader;
begin
//request sensor config..
h.PackIdent[0] := IDENT_LO;
h.PackIdent[1] := IDENT_HI;
h.Command:=CMD_CONFIG_GET;
h.DataSize :=0;
sckCam.Send(@h,sizeof(TPacketHeader));
end;

procedure TCamViewFrm.btnPreviewClick(Sender: TObject);
var
h:TPacketHeader;

begin
//ask for a jpeg..
btnPreview.Enabled := false;
btnPutConfig.Enabled := false;
btnGetConfig.Enabled := false;
h.PackIdent[0] := IDENT_LO;
h.PackIdent[1] := IDENT_HI;
h.Command  := CMD_JPG_BEGIN;
h.DataSize := 0;
sckCam.Send(@h,SizeOf(TPacketHeader));

end;

procedure TCamViewFrm.btnPutConfigClick(Sender: TObject);
var
s:TCamSensor;
h:TPacketHeader;
b:tBytes;
begin
//put sensor config..
SaveCamSensor;
s := Camera.Sensor;
h.PackIdent[0] := IDENT_LO;
h.PackIdent[1] := IDENT_HI;
h.Command  := CMD_CONFIG_SET;
h.DataSize := SizeOf(TCamSensor);
SetLength(b,SizeOf(TPacketHeader)+SizeOf(TCamSensor));
move(h,b[0],SizeOf(TPacketHeader));
move(s,b[SizeOf(TPacketHeader)],SizeOf(TCamSensor));
sckCam.Send(b,Length(b));
SetLength(b,0);
btnPreview.Enabled := true;
end;

procedure TCamViewFrm.btnRecApplyClick(Sender: TObject);
var
ndx:integer;
begin
//apply recording options..
if RecordMode <> cmbRecordMode.ItemIndex then
 begin
   RecordMode := cmbRecordMode.ItemIndex;
   if RecordMode = 1 then
     begin
      RecordFrame := true;
      tmrRecord.Interval := StrToInt(edRecInterval.Text);
     end;
     Camera.RecMode := RecordMode;
     Camera.RecIntv := StrToInt(edRecInterval.Text);
 end;
     Camera.RecIntv := StrToInt(edRecInterval.Text);

end;

procedure TCamViewFrm.btnSaveClick(Sender: TObject);
var
aFileName:String;
begin
//save image..
if haveImage then
  begin
  if dlgSavePic.Execute then
    begin
      aFileName:=dlgSavePic.FileName;
      imgCam.Picture.SaveToFile(aFileName);
      ShowMessage('Image saved..');
    end;
  end;
end;

procedure TCamViewFrm.cbSilenceAlertClick(Sender: TObject);
begin
if cbSilenceAlert.Checked then
  begin
    tmrAlert.Enabled:=false;
    AllowRemoteAlerts:= false;
  end else
    AllowRemoteAlerts := true;
end;

procedure TCamViewFrm.chkDebugClick(Sender: TObject);
begin
if chkDebug.Checked then
  begin
    mLog.Height := 400;
    Height := Height + 320;
    debugging:=true;
  end else
    begin
    mLog.Height := 80;
    if (Height-320) < 400 then
     Height := 400 else
      Height := Height - 320;
    debugging:=false;
    end;
end;

procedure TCamViewFrm.chkRecordClick(Sender: TObject);
begin
//are we saving the stream..
if chkRecord.Checked then
  begin
  //start recording process..
   StartRecording;
  end else
    begin
    //stop recording..
    StopRecording;
    end;

end;

procedure TCamViewFrm.StartRecording;
var
newFolder:String;
begin
  //setup for recording the stream

 //check for day change..
 newFolder:=FormatDateTime('YYYY-MM-DD', now);
 if vFolder <> newFolder then vFolder:=newFolder;
 {$I-}
  MkDir(vFilePath+vFolder+'\');
{$I+}

 vFileName:=FormatDateTime('HH',now)+'.mjpeg';

 if not TFile.Exists(vFilePath+vFolder+'\'+vFileName) then
 begin
 strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmCreate);
 end else
   begin
   strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmOpenReadWrite);
   strmFile.Position:=strmFile.Size;
   end;
 recording :=true;
 RecordFrame := false;
 if RecordMode = 1 then tmrRecord.Enabled:= true;

end;

procedure TCamViewFrm.SaveFrame;
var
newFileName:String;
newFolder:String;
begin
if RecordMode = 1 then
  if not RecordFrame then Exit;


  //save current buffer contents..
  if Assigned(strmFile) then
    begin
    //check for hour change
     newFileName:=FormatDateTime('HH',now)+'.mjpeg';
     if newFileName <> vFileName then
       begin
       //check for day change..
         newFolder:=FormatDateTime('YYYY-MM-DD', now);
         if vFolder <> newFolder then vFolder:=newFolder;
          {$I-}
           MkDir(vFilePath+vFolder+'\');
          {$I+}

         vFileName := newFileName;
         strmFile.FlushBuffer;
         strmFile.Free;
          if not TFile.Exists(vFilePath+vFolder+'\'+vFileName) then
           begin
             strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmCreate);
             end else
              begin
               strmFile :=TBufferedFileStream.Create(vFilePath+vFolder+'\'+vFileName,fmOpenReadWrite);
               strmFile.Position:=strmFile.Size;
               end;
       end;
    strmFile.Position:=strmFile.Size;//seek the end..
    strmFile.WriteBuffer(imgSize,4);//write the frame size
    strmFile.WriteBuffer(imgBuff[0],imgSize);//then the frame..
    end;
if RecordMode = 1 then
  begin
    RecordFrame := false;
    tmrRecord.Enabled := true;
  end;


end;

procedure TCamViewFrm.StopRecording;
begin
  //stop recording..
  recording := false;
  strmFile.FlushBuffer;
  strmFile.Free;
  strmFile := nil;
  tmrRecord.Enabled := false;
end;

//alert
procedure TCamViewFrm.tmrAlertTimer(Sender: TObject);
begin
FlashWindow(Self.Handle,true);
end;

procedure TCamViewFrm.tmrRecordTimer(Sender: TObject);
begin
//grab a frame..
tmrRecord.Enabled := false;
RecordFrame := true;
end;

procedure TCamViewFrm.tmrTimedOutTimer(Sender: TObject);
begin
close;
end;


procedure TCamViewFrm.tsAECsensorClick(Sender: TObject);
begin
if tsAECsensor.State = tssOff then
   begin

     tbExposure.Visible := false
   end else
      begin
        tbExposure.Visible := true;
      end;
end;

procedure TCamViewFrm.tsAGCClick(Sender: TObject);
begin
if tsAGC.State = tssOff then
  begin
   //enable gain dis ceiling
   tbGain.Visible := true;
   tbGainCeiling.Visible := false;
  end else
     begin
     //dis gain, enable ceiling..
      tbGain.Visible := false;
      tbGainCeiling.Visible := true;
     end;
end;

procedure TCamViewFrm.sckCamBgException(Sender: TObject; E: Exception; var CanClose: Boolean);
begin
mlog.Lines.Insert(0,E.Message);
end;

procedure TCamViewFrm.sckCamDataAvailable(Sender: TObject; ErrCode: Word);
var
Len:integer;
begin
 if debugging then
   begin
   //don't blow up..
   if mLog.Lines.Count > 999 then mLog.Lines.Clear;
   mLog.Lines.Insert(0,'*** Data Avail Error Code:'+IntToStr(ErrCode)+' Current Recv Count:'+IntToStr(RecvCount)+' ****');
   end;

   tmrTimedOut.Enabled := false;

if RecvState = 0 then
  begin
  //recv header..
  Len:=sckCam.Receive(@inBuff[RecvCount],SizeOf(TPacketHeader)-RecvCount);
  if debugging then
    mLog.Lines.Insert(0,'Header Bytes Recvd:'+IntToStr(Len));
  if Len < 0 then Len :=0;
  //got something..
  if Len > 0 then
   begin
   tmrTimedOut.Enabled := false;
     RecvCount := RecvCount + Len;
   if RecvCount = SizeOf(TPacketHeader) then
     begin
       move(inBuff[0],hdr,SizeOf(TPacketHeader));
       RecvCount := 0;
       if (hdr.PackIdent[0] = IDENT_LO) and (hdr.PackIdent[1] = IDENT_HI) then
         begin
         //valid ident..
         if hdr.DataSize > 0  then
           begin
             if hdr.DataSize > MAX_IMSIZE then
               begin
               if debugging then
                mLog.Lines.Insert(0,'Too Big:'+IntToStr(hdr.DataSize));
                 hdr.DataSize :=0;
                 RecvCount := 0;
                 sckCam.Flush;
                 exit;
               end;
             RecvCount := sckCam.Receive(@inBuff[RecvCount],hdr.DataSize);
            if RecvCount < 0 then RecvCount :=0;//zero it on error..
            if debugging then
                mLog.Lines.Insert(0,'Recvd Extra:'+IntToStr(RecvCount)+' of '+intToStr(hdr.DataSize));
             if RecvCount = hdr.DataSize then
               begin
               //got it..
               piProcessIncoming;
               //get ready for next..
               RecvCount :=0;
               RecvState :=0;
               end else
                  begin
                   RecvState := 1;//receiving more later..
                   tmrTimedOut.Enabled:=true;
                   end;
           end else
             begin
               //just a header..
               //process
               piProcessIncoming;
               //get ready for next
               RecvCount :=0;
               RecvState :=0;
             end;
         end else
           begin
             //bad ident..
                Len := sckCam.Receive(@inBuff[0],MAX_IMSIZE);
            if debugging then
                mLog.Lines.Insert(0,'Error: Bad Ident: Tossing:'+IntToStr(Len));
                sckCam.Flush;
             tmrTimedOut.Enabled := true;
           end;
     end;//we got a packet..
   end;//if we have something
  end else
    begin
     if debugging then
      mLog.Lines.Insert(0,'Remaining:'+IntToStr(hdr.DataSize - RecvCount)+' of '+intToStr(hdr.DataSize));
     Len := sckCam.Receive(@inBuff[RecvCount],hdr.DataSize-RecvCount);
     if (JPG_CHUNK_SIZE < (hdr.DataSize - RecvCount)) then
       if (Len < JPG_CHUNK_SIZE) and (Len > 0) then
         begin
          mLog.Lines.Insert(0,'**** Missing Packets ******');
         end;


     if debugging then
     mLog.Lines.Insert(0,'Recvd Chunk:'+IntToStr(Len)+' Remaining:'+intToStr(hdr.DataSize-(RecvCount+Len)));
     if Len < 0 then Len :=0;//zero on error..
     RecvCount := RecvCount + Len;
     if debugging then
     mLog.Lines.Insert(0,'RecvCount:'+IntToStr(RecvCount)+' hdr.DataSize:'+intToStr(hdr.DataSize));
     tmrTimedOut.Enabled := false;
      if RecvCount = hdr.DataSize then
      begin
        tmrTimedOut.Enabled:=false;
       //got it..
       piProcessIncoming;
       //get ready for next..
       RecvCount :=0;
       RecvState :=0;
      end else tmrTimedOut.Enabled:=true;
    end;
   tmrTimedOut.Enabled := true;
end;


procedure TCamViewFrm.piProcessIncoming;
begin
  //process incoming packet..
  if debugging then
   mLog.Lines.Insert(0,'Processing command:'+IntToStr(hdr.Command));

  case hdr.Command of
  CMD_NAK       :piProcessNak; //didn't like something..
  CMD_ACK       :piProcessAck; //acknowledge
  CMD_IDENT     :piProcessIdent; //identify
  CMD_SYNC      :piProcessSync; //sync time
  CMD_JPG_BEGIN :piProcessJpgBegin;  //getting a jpg..
  CMD_JPG_CHUNK :piProcessJpgChunk;  //getting a jpg chunk..
  CMD_JPG_END   :piProcessJpgEnd;  //got a jpg..
  CMD_CONFIG_GET:piProcessConfigGet;
  CMD_CONFIG_SET:piProcessConfigSet;
  CMD_REC_ON    :piProcessRecOn;
  CMD_REC_OFF   :piProcessRecOff;
  CMD_ALERT_ON  :piProcessAlertOn;
  CMD_ALERT_OFF :piProcessAlertOff;
  CMD_OTA_CHUNK :piProcessOtaChunk ; //send a chunk..
  end;
end;

procedure TCamViewFrm.piProcessAck;
begin
  //ack..
end;

procedure TCamViewFrm.piProcessNak;
begin
  //not acknowleged..

end;

procedure TCamViewFrm.piProcessIdent;
var
outb:tBytes;
aStr:String;
aBytes:Tbytes;
ndx:integer;
tmpId:TGuid;
begin
  //identify
  if hdr.DataSize = SizeOf(TCamId) then
   begin
   if debugging then
    mLog.Lines.Insert(0,'Processing Ident..');
    move(inBuff[0],ident,SizeOf(TCamId));
    SetLength(aBytes,SizeOf(ident.CamIdent));
    move(ident.CamIdent[0],aBytes[0],SizeOf(ident.CamIdent));
    aStr:=TEncoding.ASCII.GetString(aBytes);
    aStr:=Trim(aStr);
   if debugging then
    begin
    mLog.Lines.Insert(0,'Cam Num: '+IntToStr(ident.CamNum));
    mLog.Lines.Insert(0,'Firmware ver: '+IntToStr(ident.VerHi)+'.'+IntToStr(ident.VerLo));
    mLog.Lines.Insert(0,'');
    mLog.Lines.Insert(0,'Cam Id recvd: '+aStr );
    end;
    try
    tmpId:=StringToGUID(aStr);
    Except on e:exception do
      begin
        //got some junk??
      mLog.Lines.Insert(0,'Error:'+e.Message);
      mLog.Lines.Insert(0,aStr);
      mLog.Lines.Insert(0,'Len: '+IntToStr(Length(aStr)));
      mLog.Lines.Insert(0,'Last Val: '+IntToStr(Ord(aStr[Length(aStr)])));
      tmpID := StringToGUID('{00000000-0000-0000-0000-000000000000}');
      end;

    end;
    ndx:=CamSrvrDat.FindCam(tmpID);
    if ndx < 0 then
      begin
      //new cam
      ndx:=CamSrvrDat.AddCam;
      Camera.SetRec(CamSrvrDat.Cam[ndx].GetRec);
      if debugging then
       begin
       mLog.Lines.Insert(0,'Adding new cam id:'+GUIDToString(Camera.CamId));
       mLog.Lines.Insert(0,'Cam Num:'+IntToStr(Camera.CamNum));
       end;

      end else
         begin
         //found the cam..
         Camera.SetRec(CamSrvrDat.Cam[ndx].GetRec);
         if debugging then
          begin
          mLog.Lines.Insert(0,'Found cam id:'+GUIDToString(Camera.CamId));
          mLog.Lines.Insert(0,'Cam Num:'+IntToStr(Camera.CamNum));
          end;
         end;

     LoadCamSettings;
    ident.CamNum := Camera.CamNum;
    //GUID to array of byte..
    aStr:=GUIDToString(Camera.CamId);
    SetLength(aBytes,SizeOf(ident.CamIdent));
    aBytes:=TEncoding.ASCII.GetBytes(aStr);
    move(aBytes[0],ident.CamIdent[0],Length(aBytes));
    //
    setLength(outb,sizeof(TPacketHeader)+sizeof(TCamId));
    move(hdr,outb[0],sizeof(TPacketHeader));
    move(ident,outb[sizeof(TPacketHeader)],sizeof(TCamId));
    sckCam.Send(outb,Length(outb));
    setLength(outb,0);
    SetLength(aBytes,0);
   end;

end;

procedure TCamViewFrm.piProcessSync;
var
year, month, day, hour, minute, second, millisecond: Word;
outb:tBytes;
amnt:integer;
begin
  //sync time
  if debugging then
   mLog.Lines.Insert(0,'Processing Sync..');
   DecodeDateTime(now,year,month,day,hour,minute,second,millisecond);
   sync.Year  := Year;
   sync.day   := Day;
   sync.month := Month;
   sync.hour  := Hour;
   sync.min   := Minute;
   sync.sec   := 0;
  if debugging then
   mLog.Lines.Insert(0,'Sync Date Time: '+IntToStr(sync.Year)+'/'+IntToStr(sync.month)+
    '/'+IntToStr(sync.day)+'  '+IntToStr(sync.hour)+':'+IntToStr(sync.min));

   hdr.Command := CMD_SYNC;
   hdr.DataSize := SizeOf(TCamSync);
   setLength(outb,sizeof(TPacketHeader)+sizeof(TCamSync));
   move(hdr,outb[0],sizeof(TPacketHeader));
   move(sync,outb[sizeof(TPacketHeader)],sizeof(TCamSync));
   sckCam.Send(outb,Length(outb));
   setLength(outb,0);
end;

procedure TCamViewFrm.piProcessOtaChunk;
begin
  //ota chunk..
end;

procedure TCamViewFrm.piProcessRecOn;
begin
  //rec on
  if AllowRemoteRecord then
     StartRecording;
end;

procedure TCamViewFrm.piProcessRecOff;
begin
  //rec off
  If AllowRemoteRecord then
   StopRecording;
end;

procedure TCamViewFrm.piProcessAlertOn;
begin
  //alert on
  if AllowRemoteAlerts then
     tmrAlert.Enabled:=true;
end;

procedure TCamViewFrm.piProcessAlertOff;
begin
  //alert off
  if AllowRemoteAlerts then
     tmrAlert.Enabled:=false;
end;

procedure TCamViewFrm.piSkip;
begin
  //skipping..
  //get ready for next..
  RecvCount :=0;
  RecvState :=0;
  //ask for next frame..
  hdr.Command := CMD_JPG_BEGIN;
  hdr.DataSize := 0;
  sckCam.Send(@hdr,sizeof(TPacketHeader));
end;


procedure TCamViewFrm.piProcessJpgBegin;
begin
  //start a new jpg..
  if hdr.DataSize = sizeOf(imgSize) then
    move(inBuff[0],imgSize,SizeOf(imgSize));
  if debugging then
    mLog.Lines.Insert(0,'Begin image: '+IntToStr(imgSize));
  imgRecvd :=0;
 //request chunk..
 hdr.Command := CMD_JPG_CHUNK;
 hdr.DataSize := 0;
 sckCam.Send(@hdr,sizeof(TPacketHeader));
end;

procedure TCamViewFrm.piProcessJpgChunk;
var
sent,dsize:integer;
begin
  //save a jpg chunk..
 move(inBuff[0],imgBuff[imgRecvd],hdr.DataSize);
 imgRecvd:=imgRecvd+hdr.DataSize;
 //request another..
 hdr.Command := CMD_JPG_CHUNK;
 dsize :=hdr.DataSize;
 hdr.DataSize := 0;
 sent := sckCam.Send(@hdr,sizeof(TPacketHeader));
  if debugging then
    mLog.Lines.Insert(0,'chunk recvd: '+IntToStr(dsize)+' total recvd:'+IntToStr(imgRecvd)+' remaining:'+IntToStr(imgSize-imgRecvd)+' sent:'+IntToStr(sent));
end;



procedure TCamViewFrm.piProcessJpgEnd;
var
jpg:tJpegImage;
begin
 jpg :=nil;
 //move in last chunk..
  move(inBuff[0],imgBuff[imgRecvd],hdr.DataSize);
  imgRecvd:=imgRecvd+hdr.DataSize;

  //got a image..
       if debugging then
          mLog.Lines.Insert(0,'Recieved image:'+IntToStr(imgSize)+' byte recvd:'+IntToStr(imgRecvd));

         CamStrm.SetSize(imgSize);
         CamStrm.Position:=0;
         CamStrm.Write(imgBuff[0],imgSize);
         CamStrm.Position:=0;
           try
             jpg:=tJpegImage.Create;
             jpg.LoadFromStream(CamStrm);
             imgCam.Picture.Assign(jpg);
             haveImage:=true;
             if recording then SaveFrame;
           except on e:exception do
             begin
              mLog.Lines.Insert(0,e.Message);
              Inc(BadPacks);
              edBadPacks.Text:=IntToStr(BadPacks);
              imgSize:=0;
              RecvCount:=0;
             end;
           end;

           if Assigned(jpg) then jpg.Free;

             imgSize:=0;
 if not showingConfig then
  begin
   //ask for next frame..
   hdr.Command := CMD_JPG_BEGIN;
   hdr.DataSize := 0;
   sckCam.Send(@hdr,sizeof(TPacketHeader));
  end else//else stay paused..
    begin
      btnPreview.Enabled   := true;
      btnGetConfig.Enabled := true;
      btnPutConfig.Enabled := true;
    end;
end;

procedure TCamViewFrm.piProcessConfigGet;
var
outb:tBytes;
amnt:integer;
s:TCamSensor;

begin
  //send config..
   hdr.Command  := CMD_CONFIG_SET;
   hdr.DataSize := SizeOf(TCamSensor);
   setLength(outb,sizeof(TPacketHeader)+SizeOf(TCamSensor));
   move(hdr,outb[0],sizeof(TPacketHeader));
   s:= Camera.Sensor;
   move(s,outb[sizeof(TPacketHeader)],SizeOf(TCamSensor));
   sckCam.Send(outb,Length(outb));
   setLength(outb,0);
end;

procedure TCamViewFrm.piProcessConfigSet;
var
s:TCamSensor;
begin
    if hdr.DataSize = SizeOf(TCamSensor) then
      begin
      move(inBuff[0],s,SizeOf(TCamSensor));
      Camera.Sensor := s;
      LoadCamSensor;
      btnPutConfig.Enabled := true;
      end;
end;

procedure TCamViewFrm.sckCamError(Sender: TObject);
begin
mLog.Lines.Insert(0,'Error');
end;

procedure TCamViewFrm.sckCamException(Sender: TObject; SocExcept: ESocketException);
begin
mLog.Lines.Insert(0,'Sock Error:')
end;

procedure TCamViewFrm.sckCamSessionClosed(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Disconnected..');
Close;
end;

procedure TCamViewFrm.sckCamSessionConnected(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Connected..');
end;

procedure TCamViewFrm.sckCamSocksConnected(Sender: TObject; ErrCode: Word);
begin
mLog.Lines.Insert(0,'Socks Connected:'+IntToStr(ErrCode));
end;

procedure TCamViewFrm.sckCamSocksError(Sender: TObject; Error: Integer; Msg: string);
begin
mLog.Lines.Insert(0,'Socks Error:'+IntToStr(Error)+' '+Msg);
end;

end.
