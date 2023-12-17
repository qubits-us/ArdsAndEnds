object CamViewFrm: TCamViewFrm
  Left = 0
  Top = 0
  Caption = 'NARDS: Cam Viewer'
  ClientHeight = 489
  ClientWidth = 628
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  TextHeight = 15
  object imgCam: TImage
    Left = 0
    Top = 41
    Width = 628
    Height = 318
    Align = alClient
    Stretch = True
    ExplicitLeft = 88
    ExplicitTop = 104
    ExplicitWidth = 105
    ExplicitHeight = 105
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 628
    Height = 41
    Align = alTop
    TabOrder = 0
    object edBadPacks: TEdit
      Left = 529
      Top = 12
      Width = 74
      Height = 23
      ReadOnly = True
      TabOrder = 0
      Text = '0'
    end
  end
  object pblBottom: TPanel
    Left = 0
    Top = 448
    Width = 628
    Height = 41
    Align = alBottom
    TabOrder = 1
    object btnClose: TButton
      Left = 528
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Close'
      TabOrder = 0
      OnClick = btnCloseClick
    end
    object btnDiscon: TButton
      Left = 8
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Disconnect'
      TabOrder = 1
      OnClick = btnDisconClick
    end
    object btnSave: TButton
      Left = 447
      Top = 8
      Width = 75
      Height = 25
      Caption = 'Save'
      TabOrder = 2
      OnClick = btnSaveClick
    end
    object chkDebug: TCheckBox
      Left = 320
      Top = 10
      Width = 97
      Height = 17
      Caption = 'Show Debug'
      TabOrder = 3
      OnClick = chkDebugClick
    end
  end
  object mLog: TMemo
    Left = 0
    Top = 359
    Width = 628
    Height = 89
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
