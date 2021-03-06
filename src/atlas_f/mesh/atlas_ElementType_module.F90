#include "atlas/atlas_f.h"

module atlas_ElementType_module

use fckit_owned_object_module, only : fckit_owned_object

implicit none

private :: fckit_owned_object

public :: atlas_ElementType
public :: atlas_Triangle
public :: atlas_Quadrilateral
public :: atlas_Line


private

!-----------------------------
! atlas_ElementType     !
!-----------------------------

type, extends(fckit_owned_object) :: atlas_ElementType
contains
! Public methods
  procedure, public :: nb_nodes
  procedure, public :: nb_edges
  procedure, public :: name
  procedure, public :: parametric
#if FCKIT_FINAL_NOT_INHERITING
  final :: atlas_ElementType__final_auto
#endif
end type

interface atlas_ElementType
  module procedure atlas_ElementType__cptr
end interface

interface atlas_Triangle
  module procedure atlas_Triangle__constructor
end interface

interface atlas_Quadrilateral
  module procedure atlas_Quadrilateral__constructor
end interface

interface atlas_Line
  module procedure atlas_Line__constructor
end interface

!========================================================
contains
!========================================================

function atlas_ElementType__cptr(cptr) result(this)
  use, intrinsic :: iso_c_binding, only : c_ptr
  use atlas_elementtype_c_binding
  type(atlas_ElementType) :: this
  type(c_ptr) :: cptr
  call this%reset_c_ptr( cptr )
  call this%return()
end function

function atlas_Quadrilateral__constructor() result(this)
  use atlas_elementtype_c_binding
  type(atlas_ElementType) :: this
  call this%reset_c_ptr( atlas__mesh__Quadrilateral__create() )
  call this%return()
end function

function atlas_Triangle__constructor() result(this)
  use atlas_elementtype_c_binding
  type(atlas_ElementType) :: this
  call this%reset_c_ptr( atlas__mesh__Triangle__create() )
  call this%return()
end function

function atlas_Line__constructor() result(this)
  use atlas_elementtype_c_binding
  type(atlas_ElementType) :: this
  call this%reset_c_ptr( atlas__mesh__Line__create() )
  call this%return()
end function

function nb_nodes(this)
  use, intrinsic :: iso_c_binding, only : c_size_t
  use atlas_elementtype_c_binding
  integer(c_size_t) :: nb_nodes
  class(atlas_ElementType), intent(in) :: this
  nb_nodes = atlas__mesh__ElementType__nb_nodes(this%c_ptr())
end function

function nb_edges(this)
  use, intrinsic :: iso_c_binding, only : c_size_t
  use atlas_elementtype_c_binding
  integer(c_size_t) :: nb_edges
  class(atlas_ElementType), intent(in) :: this
  nb_edges = atlas__mesh__ElementType__nb_edges(this%c_ptr())
end function

function name(this)
  use, intrinsic :: iso_c_binding, only : c_ptr
  use fckit_c_interop_module, only : c_ptr_to_string
  use atlas_elementtype_c_binding
  character(len=:), allocatable :: name
  class(atlas_ElementType) :: this
  type(c_ptr) :: name_c_str
  name_c_str = atlas__mesh__ElementType__name(this%c_ptr())
  name = c_ptr_to_string(name_c_str)
end function

function parametric(this)
  use, intrinsic :: iso_c_binding, only : c_int
  use atlas_elementtype_c_binding
  logical :: parametric
  class(atlas_ElementType), intent(in) :: this
  integer(c_int) :: parametric_int
  parametric_int = atlas__mesh__ElementType__parametric(this%c_ptr())
  if( parametric_int == 0 ) then
    parametric = .False.
  else
    parametric = .True.
  endif
end function

!-------------------------------------------------------------------------------

subroutine atlas_ElementType__final_auto(this)
  type(atlas_ElementType) :: this
#if FCKIT_FINAL_DEBUGGING
  write(0,*) "atlas_ElementType__final_auto"
#endif
#if FCKIT_FINAL_NOT_PROPAGATING
  call this%final()
#endif
  FCKIT_SUPPRESS_UNUSED( this )
end subroutine

end module atlas_ElementType_module

