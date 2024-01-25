unit frmReplay;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ExtCtrls,Vcl.Imaging.jpeg,
  Vcl.ComCtrls;

type
    TVideoFrames = record
      ImSize:integer;
      ImPosition:integer;
    end;


type
  TReplayFrm = class(TForm)
    pnlTop: TPanel;
    im: TImage;
    edFileName: TEdit;
    btnSelect: TButton;
    lblSelectFile: TLabel;
    btnPlay: TButton;
    btnStop: TButton;
    btnPrev: TButton;
    btnNext: TButton;
    btnSavePic: TButton;
    btnClose: TButton;
    dlgSelect: TOpenDialog;
    tmrPlayback: TTimer;
    lblFrameInfo: TLabel;
    pbFrames: TProgressBar;
    procedure btnSelectClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure tmrPlaybackTimer(Sender: TObject);
    procedure LoadFile;
    procedure ShowFrame;
    procedure btnCloseClick(Sender: TObject);
    procedure btnNextClick(Sender: TObject);
    procedure btnPrevClick(Sender: TObject);
    procedure btnPlayClick(Sender: TObject);
    procedure btnStopClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    Playing:boolean;
    FileLoaded : boolean;
    strmFile:TFileStream;
    ImgSize:Int32;
    strmImg:TMemoryStream;
    frames:array of tVideoFrames;
    CurrFrame,NumFrames:integer;
  end;

var
  ReplayFrm: TReplayFrm;

implementation

{$R *.dfm}

procedure TReplayFrm.btnCloseClick(Sender: TObject);
begin
ModalResult :=mrOK;
end;

procedure TReplayFrm.btnNextClick(Sender: TObject);
begin
//load next frame..
if CurrFrame < NumFrames-1 then
  begin
    Inc(CurrFrame);
    ShowFrame;
  end;
end;



procedure TReplayFrm.btnPlayClick(Sender: TObject);
begin
//play
if (FileLoaded) and (not Playing)  then
  begin
    Playing := true;
    tmrPlayback.Enabled:=true;
    btnPrev.Enabled:=false;
    btnNext.Enabled:=false;
    btnSavePic.Enabled:=false;
    btnSelect.Enabled:=false;
  end;


end;

procedure TReplayFrm.btnStopClick(Sender: TObject);
begin
if Playing then
  begin
    Playing := false;
    tmrPlayback.Enabled:=false;
    btnPrev.Enabled:=true;
    btnNext.Enabled:=true;
    btnSavePic.Enabled:=true;
    btnSelect.Enabled:=true;
  end;
end;


procedure TReplayFrm.btnPrevClick(Sender: TObject);
begin
//load prev frame..
if CurrFrame > 0 then
  begin
    Dec(CurrFrame);
    ShowFrame;
  end;

end;

procedure TReplayFrm.ShowFrame;
var
jpg:TJpegImage;
begin
  //show the current frame..
          strmImg.SetSize(frames[CurrFrame].ImSize);
          strmImg.Position:=0;
          strmFile.Position:=frames[CurrFrame].ImPosition;
          strmImg.CopyFrom(strmFile,frames[CurrFrame].ImSize);
          strmImg.Position:=0;
          jpg :=tJpegImage.Create;
          try
            jpg.LoadFromStream(strmImg);
            im.Picture.Assign(jpg);
          finally
          jpg.Free;
          end;
      pbFrames.Position:=CurrFrame+1;
      lblFrameInfo.Caption:=IntToStr(CurrFrame+1)+' of '+IntToStr(NumFrames);
end;

procedure TReplayFrm.btnSelectClick(Sender: TObject);
var
jpg:TJpegImage;
begin
if dlgSelect.Execute then
  begin
    edFileName.Text:=dlgSelect.FileName;
    tmrPlayback.Enabled:=false;
    Playing := false;
    FileLoaded := false;
    if Assigned(strmFile) then strmFile.Free;
    strmFile:=tFileStream.Create(edFileName.Text,fmOpenRead);
    if strmFile.Size>0 then
      begin
      strmFile.Position:=0;
      strmFile.ReadBuffer(ImgSize,4);
      if (ImgSize > 0) and (ImgSize < strmFile.Size) then
        begin
          strmImg.SetSize(ImgSize);
          strmImg.Position:=0;
          strmImg.CopyFrom(strmFile,ImgSize);
          strmImg.Position:=0;
          jpg :=tJpegImage.Create;
          try
            jpg.LoadFromStream(strmImg);
            im.Picture.Assign(jpg);
          finally
          jpg.Free;
          end;
        LoadFile;
        end;
      end;
  end;
end;


procedure TReplayFrm.LoadFile;
var
imgCount,i:integer;
imSize:int32;
done,err:boolean;
begin

done := false;
err := false;
strmFile.Position:=0;
imgCount := 0;
i:=0;

pbFrames.Position :=0;

Screen.Cursor := crHourGlass;

while not done do
  begin
   try
   strmFile.ReadBuffer(ImSize,4);
    if (ImSize > 0) and ((imSize+strmFile.Position) <= strmFile.Size) then
    begin
    inc(imgCount);
    strmFile.Position:=strmFile.Position+imSize;
    end else done := true;
    if strmFile.Position = strmFile.Size then done := true;
   Except on e:Exception do
     begin
      Screen.Cursor:=crDefault;
      ShowMessage(e.Message);
      done := true;
      err := true;
     end;
   end;

  end;


if not err then
   begin
   pbFrames.Max:=imgCount;
   setLength(frames,imgCount);
   strmFile.Position:=0;
   for I := 0 to imgCount -1 do
       begin
        strmFile.ReadBuffer(ImSize,4);
        frames[i].ImSize := ImSize;
        frames[i].ImPosition := strmFile.Position;
        strmFile.Position := strmFile.Position+ImSize;
        pbFrames.Position:=i;
       end;
   end;
 pbFrames.Position:=CurrFrame+1;
 NumFrames := imgCount;
 CurrFrame := 0;
 FileLoaded := true;
 lblFrameInfo.Caption:=IntToStr(CurrFrame+1)+' of '+IntToStr(NumFrames);
 Screen.Cursor:=crDefault;
end;

procedure TReplayFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
//all done..
tmrPlayback.Enabled:=false;
Playing := false;
if Assigned(strmFile) then
  strmFile.Free;
if Assigned(strmImg) then
 strmImg.Free;
//free mem..
setLength(frames,0);
end;

procedure TReplayFrm.FormCreate(Sender: TObject);
begin
//in the beginning..
Playing := false;
strmFile := nil;
strmImg := tMemoryStream.Create;
end;

procedure TReplayFrm.tmrPlaybackTimer(Sender: TObject);
begin
//display a frame
tmrPlayback.Enabled:=false;
if FileLoaded then
  begin
   if CurrFrame < NumFrames-1 then
    begin
     Inc(CurrFrame);
     ShowFrame;
    end else
     begin
      CurrFrame :=0;
      ShowFrame;
     end;
   if Playing then
   tmrPlayback.Enabled:=true;
  end;
end;
end.
