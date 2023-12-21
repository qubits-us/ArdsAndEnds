object CamViewFrm: TCamViewFrm
  Left = 0
  Top = 0
  Caption = 'ESP Cam :'
  ClientHeight = 506
  ClientWidth = 618
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDefault
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 15
  object imgCam: TImage
    Left = 0
    Top = 41
    Width = 618
    Height = 336
    Align = alClient
    Stretch = True
    ExplicitWidth = 628
    ExplicitHeight = 320
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 618
    Height = 41
    Align = alTop
    TabOrder = 0
    ExplicitWidth = 616
    DesignSize = (
      618
      41)
    object edBadPacks: TEdit
      Left = 529
      Top = 12
      Width = 74
      Height = 23
      Anchors = [akTop, akRight]
      ReadOnly = True
      TabOrder = 0
      Text = '0'
    end
  end
  object pblBottom: TPanel
    Left = 0
    Top = 465
    Width = 618
    Height = 41
    Align = alBottom
    TabOrder = 1
    ExplicitWidth = 616
    DesignSize = (
      618
      41)
    object btnClose: TButton
      Left = 528
      Top = 8
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Close'
      TabOrder = 0
      OnClick = btnCloseClick
    end
    object btnSave: TButton
      Left = 10
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Save'
      TabOrder = 1
      OnClick = btnSaveClick
    end
    object chkDebug: TCheckBox
      Left = 100
      Top = 10
      Width = 97
      Height = 17
      Caption = 'Show Debug'
      TabOrder = 2
      OnClick = chkDebugClick
    end
    object chkRecord: TCheckBox
      Left = 200
      Top = 10
      Width = 97
      Height = 17
      Caption = 'Record'
      TabOrder = 3
      OnClick = chkRecordClick
    end
  end
  object mLog: TMemo
    Left = 0
    Top = 377
    Width = 618
    Height = 88
    Align = alBottom
    Color = clBlack
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clAqua
    Font.Height = -12
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 2
    StyleElements = [seBorder]
    ExplicitWidth = 616
  end
  object sckCam: TWSocket
    LineEnd = #13#10
    Port = '12001'
    Proto = 'tcp'
    LocalAddr = '0.0.0.0'
    LocalAddr6 = '::'
    LocalPort = '0'
    SocksLevel = '5'
    ExclusiveAddr = False
    ComponentOptions = []
    ListenBacklog = 15
    OnDataAvailable = sckCamDataAvailable
    OnSessionClosed = sckCamSessionClosed
    OnSessionConnected = sckCamSessionConnected
    OnSocksConnected = sckCamSocksConnected
    OnError = sckCamError
    OnBgException = sckCamBgException
    OnSocksError = sckCamSocksError
    SocketErrs = wsErrTech
    onException = sckCamException
    Left = 528
    Top = 168
  end
  object dlgSavePic: TSavePictureDialog
    Title = 'Same Pic'
    Left = 504
    Top = 256
  end
end
