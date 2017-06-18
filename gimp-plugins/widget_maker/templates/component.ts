import {
  ChangeDetectionStrategy,
  Component,
  ElementRef,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';

export class Point {
  constructor(public x: number, public y: number) {
  }

  add(translation: Point): Point {
    return new Point(this.x + translation.x, this.y + translation.y);
  }

  subtract(translation: Point): Point {
    return new Point(this.x - translation.x, this.y - translation.y);
  }

  rotate(angle: number) {
    return new Point(
      this.x * Math.cos(angle) - this.y * Math.sin(angle),
      this.y * Math.cos(angle) + this.x * Math.sin(angle)
    );
  }
}

export interface ElementDragEvent {
  mouseDown     : Point,  // point where the drag event has been initiated
  lastCursor    : Point,  // point where that cursor was at the last fired drag event
  cursor        : Point,  // current cursor position
  translationX ?: number, // desired translation (-1.0 ... 1.0) of the MOVE layer in X direction
  translationY ?: number, // desired translation (-1.0 ... 1.0) of the MOVE layer in Y direction
  rotation     ?: number  // desired rotation (0.0 ... 1.0) of the ROTATE layer
}

class InteractiveElement {
  public image         : HTMLImageElement;
  public canvas        : HTMLCanvasElement;
  public canvasContext : CanvasRenderingContext2D;
  public cursorOnHover : string;

  constructor(interactionType: string, imgViewChild: ElementRef, canvasViewChild: ElementRef,
    public mouseDownEvent    : EventEmitter<void>,
    public mouseUpEvent      : EventEmitter<void>,
    public doubleClickEvent ?: EventEmitter<void>,
    public dragEvent        ?: EventEmitter<ElementDragEvent>,
    public dragHandler      ?: (event: MouseEvent) => void)
  {
    switch (interactionType) {
      case 'pushable':
        this.cursorOnHover = 'pointer';
        break;

      case 'grabbable':
        this.cursorOnHover = '-webkit-grab';
        break;
    }

    this.image = imgViewChild.nativeElement as HTMLImageElement;
    this.canvas = canvasViewChild.nativeElement as HTMLCanvasElement;
    this.canvasContext = this.canvas.getContext('2d');
  }
}

@Component({
  selector: '$COMPONENT_NAME$',
  templateUrl: '$COMPONENT_HTML_FILENAME$',
  styleUrls: ['$COMPONENT_CSS_FILENAME$'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class $COMPONENT_CLASS_NAME$ implements OnInit {
  @Input() blurred = false;
  $COMPONENT_PROPERTIES$

  private cursor = 'default';
  private interactiveElements: InteractiveElement[] = [];
  private activeElement: InteractiveElement = null;
  private mouseDownPoint : Point|void = null;

  ngOnInit() {
    this.interactiveElements = [];
    $NG_ON_INIT_BODY$
  }

  addInteractiveElement(ie: InteractiveElement)
  {
    ie.image.onload = () => {
      ie.canvas.width = ie.image.width;
      ie.canvas.height = ie.image.height;
      ie.canvasContext.drawImage(ie.image, 0, 0);
    };

    this.interactiveElements.push(ie);
  }

  onDragStart(event: DragEvent) {
    return false; // do not allow dragging
  }

  onMouseMove(event: MouseEvent) {
    let ae = this.activeElement;
    if (ae) {
      if (ae.dragHandler) {
        ae.dragHandler(event);
      }
    }
    else {
      let ie = this.getInteractiveElementUnderCursor(event);
      this.cursor = ie ? ie.cursorOnHover : 'default';
    }
  }

  onMouseOut(event: MouseEvent) {
    if (this.activeElement && !this.isNodeOnComponent(event.toElement)) {
      this.onMouseUp(event);
    }
  }

  onMouseDown(event: MouseEvent) {
    let ie = this.getInteractiveElementUnderCursor(event);
    if (ie) {
      this.mouseDownPoint = new Point(event.clientX, event.clientY).subtract(this.getComponentOffset());
      this.activeElement = ie;
      ie.mouseDownEvent.emit();
    }
  }

  onMouseUp(event: MouseEvent) {
    if (this.activeElement) {
      this.activeElement.mouseUpEvent.emit();
      this.activeElement = null;
      this.mouseDownPoint = null;
    }
  }

  onDoubleClick(event: MouseEvent) {
    let ie = this.getInteractiveElementUnderCursor(event);
    if (ie && ie.doubleClickEvent) {
      this.mouseDownPoint = new Point(event.clientX, event.clientY).subtract(this.getComponentOffset());
      ie.doubleClickEvent.emit();
    }
  }

  isNodeOnComponent(node: Node): boolean {
    while (node) {
      if (node === this.componentViewChild.nativeElement) {
        return true;
      }

      node = node.parentNode;
    }

    return false;
  }

  getOpacity(min: number, set: number, max: number, layerOpacity: number): number {
    return Math.min(Math.max(min, set), max) * layerOpacity;
  }

  getTransformOrigin(computedStyle: CSSStyleDeclaration): Point {
    let parts = computedStyle['transformOrigin'].split('px');
    let x = parseFloat(parts[0]);
    let y = parseFloat(parts[1]);
    return new Point(x, y);
  }

  getTranslationAndRotation(computedStyle: CSSStyleDeclaration): [Point, number] {
    let matrix = computedStyle['transform'];
    if (matrix === 'none') {
      return [new Point(0, 0), 0];
    }

    let valStrings = matrix.split('(')[1].split(')')[0].split(',');

    let translation = new Point(parseFloat(valStrings[4]), parseFloat(valStrings[5]));
    let [a, b] = [parseFloat(valStrings[0]), parseFloat(valStrings[1])];
    let angle = Math.atan2(b, a);

    return [translation, angle];
  }

  getComponentOffset(): Point {
    let clientRect = this.componentViewChild.nativeElement.getBoundingClientRect();
    return new Point(clientRect.left, clientRect.top);
  }

  getMouseEventCoordinatesForCanvas(canvas: HTMLCanvasElement, event: MouseEvent): Point {
    let eventOrigin = new Point(event.clientX, event.clientY).subtract(this.getComponentOffset());
    let canvasOffset = new Point(canvas.offsetLeft, canvas.offsetTop);

    let canvasComputedStyle = window.getComputedStyle(canvas);
    let [canvasTranslation, canvasRotation] = this.getTranslationAndRotation(canvasComputedStyle);
    let transformOrigin = this.getTransformOrigin(canvasComputedStyle);

    let eventOriginOnCanvas = eventOrigin.
      subtract(transformOrigin).
      subtract(canvasOffset).
      rotate(-canvasRotation).
      add(transformOrigin).
      subtract(canvasTranslation);

    return eventOriginOnCanvas;
  }

  isCanvasPixelUnderCursorTransparent(canvas: HTMLCanvasElement, event: MouseEvent): boolean {
    let context = canvas.getContext('2d');
    let coordinates = this.getMouseEventCoordinatesForCanvas(canvas, event);
    return context.getImageData(coordinates.x, coordinates.y, 1, 1).data[3] === 0;
  }

  getInteractiveElementUnderCursor(event: MouseEvent): InteractiveElement|void {
    let origIdx = this.interactiveElements.findIndex((ie) => ie.canvas === event.target);

    for (let idx = origIdx; idx !== -1; --idx) {
      let ie = this.interactiveElements[idx];
      if (!this.isCanvasPixelUnderCursorTransparent(ie.canvas, event)) {
        return ie;
      }
    }

    return undefined;
  }

  makeElementDragEvent(event: MouseEvent): ElementDragEvent {
    let eventOrigin = new Point(event.clientX, event.clientY).subtract(this.getComponentOffset());

    return {
      'mouseDown': this.mouseDownPoint as Point,
      'lastCursor': eventOrigin.subtract(new Point(event.movementX, event.movementY)),
      'cursor': eventOrigin
    };
  }

  calculateDesiredTranslation(delta: number, min: number, max: number, translation: number): number {
    let curTransInPx = translation * (translation > 0 ? max : -min);
    let desiredTransInPx = curTransInPx + delta;
    let desiredTrans = desiredTransInPx / (desiredTransInPx > 0 ? max : -min);

    if (desiredTrans < -1.0) {
      return -1.0;
    }
    else if (desiredTrans > 1.0) {
      return 1.0;
    }
    else {
      return desiredTrans;
    }
  }

  makeStdAngle(angle: number): number {
    while (angle > 360) {
        angle -= 360;
      }

      while (angle < 0) {
        angle += 360;
      }

      return angle;
  }

  handleDragEventForNoLayer(event: MouseEvent): void {
    let ede = this.makeElementDragEvent(event);
    this.activeElement.dragEvent.emit(ede);
  }

  handleDragEventForMoveLayer(event: MouseEvent, minX: number, maxX: number, translationX: number,
    minY: number, maxY: number, translationY: number): void
  {
    let ede = this.makeElementDragEvent(event);

    ede.translationX = this.calculateDesiredTranslation(ede.cursor.x - ede.lastCursor.x, minX, maxX, translationX);
    ede.translationY = this.calculateDesiredTranslation(ede.cursor.y - ede.lastCursor.y, minY, maxY, translationY);

    this.activeElement.dragEvent.emit(ede);
  }

  handleDragEventForRotateLayer(event: MouseEvent, centerX: number, centerY: number,
    minAngle: number, maxAngle: number): void
  {
    let ede = this.makeElementDragEvent(event);

    let dX = ede.cursor.x - centerX;
    let dY = ede.cursor.y - centerY;

    let angle = this.makeStdAngle(Math.atan2(dY, dX) * 180 / Math.PI + 90);
    let angleFromMin = this.makeStdAngle(angle - minAngle);
    let maxAngleFromMin = this.makeStdAngle(maxAngle - minAngle);

    if (angleFromMin < maxAngleFromMin) {
      ede.rotation = angleFromMin / maxAngleFromMin;
    }
    else {
      let stdMinAngle = this.makeStdAngle(minAngle);
      let stdMaxAngle = this.makeStdAngle(maxAngle);

      let distFromMin = Math.abs(stdMinAngle - angle);
      let distFromMax = Math.abs(stdMaxAngle - angle);

      ede.rotation = distFromMin < distFromMax ? 0 : 1;
    }

    this.activeElement.dragEvent.emit(ede);
  }
}
