import {
  Component,
  Input,
  Output,
  OnInit,
  OnChanges,
  SimpleChanges,
} from '@angular/core';

import {
  YogiService,
  ErrorDisplayService,
  KnownTerminalsChangedHandler,
} from '../../../core/core.module';

import {
  naturalSort,
  makeTerminalTypeString,
  makeSignatureString
} from '../../../helpers/helpers.barrel';

interface ChildInfo {
  path: string;
  subtreeInfo: yogi.SubtreeInfo;
}

@Component({
  selector: 'ce-tree',
  templateUrl: 'tree.component.html',
  styleUrls: ['tree.component.css']
})
export class TreeComponent implements OnInit, OnChanges {
  @Input() path: string;
  @Input() rootName: string;
  @Input() subtreeInfo: yogi.SubtreeInfo;
  @Input() expandAll: boolean = false;

  private name: string;
  private isRoot: boolean;
  private isFolder: boolean;
  private rootIsShown: boolean;
  private children: ChildInfo[] = [];
  private childrenLoading: boolean = false;
  private childrenLoaded: boolean = false;
  private expanded: boolean;

  constructor(private YogiService: YogiService,
              private errorDisplayService: ErrorDisplayService) {
  }

  ngOnInit() {
    this.setupGuiVariables();

    if (this.expanded) {
      this.loadChildren();
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    if ('subtreeInfo' in changes) {
      this.isFolder = !this.subtreeInfo.terminals.length;
    }
  }

  setupGuiVariables() {
    this.name = this.path.substr(this.path.lastIndexOf('/') + 1);
    this.isRoot = typeof this.subtreeInfo === 'undefined';
    this.rootIsShown = this.isRoot && typeof this.rootName === 'string';
    this.expanded = this.isRoot || this.expandAll;
  }

  loadChildren() {
    this.childrenLoading = true;
    this.YogiService.knownTerminalsObserver.getSubtree(this.path)
    .then((infos) => {
      this.onChildrenReceived(infos);
    })
    .catch((err) => {
      this.errorDisplayService.show(`Retrieving Terminal subtree for path '${this.path}' failed`,
        err);
    });
  }

  removeChildren() {
    this.children = [];
    this.childrenLoading = false;
    this.childrenLoaded = false;
  }

  addChildren(infos: yogi.SubtreeInfo[]) {
    let prefix = this.path;
    if (prefix.length > 0 && !prefix.endsWith('/')) {
      prefix += '/';
    }

    for (let info of infos) {
      let child = this.children.find((chld) => chld.subtreeInfo.name === info.name);
      if (child) {
        for (let tm of info.terminals) {
          if (this.findTerminalIndex(child.subtreeInfo, tm) === -1) {
            child.subtreeInfo.terminals.push(tm);
          }
        }
      }
      else {
        this.children.push({
          path: prefix + info.name,
          subtreeInfo: info
        });
      }
    }

    this.sortChildren();
  }

  sortChildren() {
    this.children.sort((a: ChildInfo, b: ChildInfo): number => {
      let aIsFolder = a.subtreeInfo.terminals.length === 0;
      let bIsFolder = b.subtreeInfo.terminals.length === 0;
      if (aIsFolder !== bIsFolder) {
        return aIsFolder ? -1 : +1;
      }

      return naturalSort(a.subtreeInfo.name, b.subtreeInfo.name);
    });
  }

  onChildrenReceived(infos: yogi.SubtreeInfo[]) {
    if (!this.expanded) {
      this.childrenLoading = false;
      return;
    }

    this.addChildren(infos);

    this.childrenLoading = false;
    this.childrenLoaded = true;
  }

  onClicked() {
    this.expanded = !this.expanded;
    if (this.expanded) {
      this.loadChildren();
    }
    else {
      this.removeChildren();
    }
  }

  findTerminalIndex(subtreeInfo: yogi.SubtreeInfo,
                    terminalInfo: yogi.NamelessTerminalInfo): number {
    return subtreeInfo.terminals.findIndex((tm) => {
      return terminalInfo.type === tm.type && terminalInfo.signature.raw === tm.signature.raw;
    });
  }

  makeTerminalIcon(terminalType: any): string {
    switch (terminalType) {
      case yogi.DeafMuteTerminal:
        return 'fa-tag';

      case yogi.PublishSubscribeTerminal:
        return 'fa-comment';

      case yogi.CachedPublishSubscribeTerminal:
        return 'fa-commenting';

      case yogi.ScatterGatherTerminal:
        return 'fa-reply-all';

      case yogi.ProducerTerminal:
      case yogi.CachedProducerTerminal:
        return 'fa-bullhorn';

      case yogi.MasterTerminal:
      case yogi.CachedMasterTerminal:
        return 'fa-sitemap';

      case yogi.ServiceTerminal:
        return 'fa-weixin';

      default:
        throw new Error(`Invalid terminal type: ${terminalType}.`);
    }
  }

  makeTerminalHeaderTooltip(terminal: yogi.NamelessTerminalInfo): string {
    return [
      makeTerminalTypeString(terminal.type),
      this.path,
      makeSignatureString(terminal.signature, terminal.type)
    ].join('\n');
  }
}
