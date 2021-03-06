#include "atlas/atlas_f.h"

module atlas_connectivity_module

use, intrinsic :: iso_c_binding, only : c_int, c_size_t, c_ptr, c_null_ptr
use fckit_owned_object_module, only : fckit_owned_object
use fckit_object_module, only : fckit_object
implicit none

private :: c_ptr
private :: c_int
private :: c_size_t
private :: c_null_ptr
private :: fckit_owned_object
private :: fckit_object

public :: atlas_Connectivity
public :: atlas_MultiBlockConnectivity
public :: atlas_BlockConnectivity


private

!-----------------------------
! atlas_Connectivity         !
!-----------------------------

type, extends(fckit_owned_object) :: atlas_Connectivity

! Public members
  type( atlas_ConnectivityAccess ), public, pointer :: access => null()

contains

  procedure, public :: assignment_operator_hook
  procedure, public :: set_access

! Public methods
  procedure, public  :: name => atlas_Connectivity__name
  procedure, private :: value_args_int       => atlas_Connectivity__value_args_int
  procedure, private :: value_args_size_t    => atlas_Connectivity__value_args_size_t
  generic, public :: value => value_args_int, value_args_size_t
  procedure, public :: rows     => atlas_Connectivity__rows
  procedure, public :: cols     => atlas_Connectivity__cols
  procedure, public :: maxcols  => atlas_Connectivity__maxcols
  procedure, public :: mincols  => atlas_Connectivity__mincols
  procedure, public :: padded_data     => atlas_Connectivity__padded_data
  procedure, public :: data     => atlas_Connectivity__data
  procedure, public :: row      => atlas_Connectivity__row
  procedure, public :: missing_value  => atlas_Connectivity__missing_value
  procedure, private :: add_values_args_int => atlas_Connectivity__add_values_args_int
  procedure, private :: add_values_args_size_t => atlas_Connectivity__add_values_args_size_t
  procedure, private :: add_missing_args_int => atlas_Connectivity__add_missing_args_int
  procedure, private :: add_missing_args_size_t => atlas_Connectivity__add_missing_args_size_t
  generic, public :: add => add_values_args_int, add_values_args_size_t, add_missing_args_int, add_missing_args_size_t

#if FCKIT_FINAL_NOT_INHERITING
  final :: atlas_Connectivity__final_auto
#endif

end type

!---------------------------------------
! atlas_MultiBlockConnectivity         !
!---------------------------------------

type, extends(atlas_Connectivity) :: atlas_MultiBlockConnectivity
contains
! Public methods
  procedure, public :: blocks   => atlas_MultiBlockConnectivity__blocks
  procedure, public :: block    => atlas_MultiBlockConnectivity__block

! PGI compiler bug won't accept "assignment_operator_hook" from atlas_Connectivity parent class... grrr
  procedure, public :: assignment_operator_hook => atlas_MultiBlockConnectivity__assignment_operator_hook

#if FCKIT_FINAL_NOT_INHERITING
  final :: atlas_MultiBlockConnectivity__final_auto
#endif

end type

!----------------------------!
! atlas_BlockConnectivity    !
!----------------------------!

type, extends(fckit_object) :: atlas_BlockConnectivity
contains
  procedure, public :: rows     => atlas_BlockConnectivity__rows
  procedure, public :: cols     => atlas_BlockConnectivity__cols
  procedure, public :: data     => atlas_BlockConnectivity__data
  procedure, public :: missing_value  => atlas_BlockConnectivity__missing_value
end type

interface atlas_Connectivity
  module procedure Connectivity_cptr
  module procedure Connectivity_constructor
end interface

interface atlas_MultiBlockConnectivity
  module procedure MultiBlockConnectivity_cptr
  module procedure MultiBlockConnectivity_constructor
end interface

interface atlas_BlockConnectivity
  module procedure BlockConnectivity_cptr
end interface


!-------------------------------
! Helper types                 !
!-------------------------------

type :: atlas_ConnectivityAccessRow
  integer, pointer :: col(:)
contains
end type

type :: atlas_ConnectivityAccess
  integer(c_int),    private, pointer :: values_(:) => null()
  integer(c_size_t), private, pointer :: displs_(:) => null()
  integer(c_size_t), public,  pointer :: cols(:)    => null()
  type(atlas_ConnectivityAccessRow), public, pointer :: row(:) => null()
  integer(c_size_t), private :: rows_
  integer, private, pointer :: padded_(:,:) => null()
  integer(c_size_t), private :: maxcols_, mincols_
  integer(c_int), private :: missing_value_
  type(c_ptr), private :: connectivity_ptr = c_null_ptr
contains
  procedure, public :: rows  => access_rows
  procedure, public :: value => access_value
end type

! ----------------------------------------------------------------------------
interface
    subroutine atlas__connectivity__register_ctxt(This,ptr) bind(c,name="atlas__connectivity__register_ctxt")
        use, intrinsic :: iso_c_binding, only: c_ptr
        type(c_ptr),    value :: This
        type(c_ptr),    value :: ptr
    end subroutine

    subroutine atlas__connectivity__register_update(This,funptr) bind(c,name="atlas__connectivity__register_update")
        use, intrinsic :: iso_c_binding, only: c_ptr, c_funptr
        type(c_ptr),    value :: This
        type(c_funptr), value :: funptr
    end subroutine

    subroutine atlas__connectivity__register_delete(This,funptr) bind(c,name="atlas__connectivity__register_delete")
        use, intrinsic :: iso_c_binding, only: c_ptr, c_funptr
        type(c_ptr),    value :: This
        type(c_funptr), value :: funptr
    end subroutine

    function atlas__connectivity__ctxt(This,ptr) bind(c,name="atlas__connectivity__ctxt")
        use, intrinsic :: iso_c_binding, only: c_ptr, c_int
        integer(c_int) :: atlas__connectivity__ctxt
        type(c_ptr),    value :: This
        type(c_ptr) :: ptr
    end function

end interface

contains
  
subroutine assignment_operator_hook(this,other)
  class(atlas_Connectivity) :: this
  class(fckit_owned_object) :: other
  call this%set_access()
  FCKIT_SUPPRESS_UNUSED(other)
end subroutine

! Following routine is exact copy of "assignment_operator_hook" above, because of bug in PGI compiler (17.7)
! Without it, wrongly the "fckit_owned_object::assignment_operator_hook" is used instead of
! "atlas_Connectivity::assignment_operator_hook".
subroutine atlas_MultiBlockConnectivity__assignment_operator_hook(this,other)
  class(atlas_MultiBlockConnectivity) :: this
  class(fckit_owned_object) :: other
  call this%set_access()
  FCKIT_SUPPRESS_UNUSED(other)
end subroutine

function Connectivity_cptr(cptr) result(this)
  use, intrinsic :: iso_c_binding, only : c_ptr
  use atlas_connectivity_c_binding
  type(atlas_Connectivity) :: this
  type(c_ptr) :: cptr
  call this%reset_c_ptr( cptr )
  call this%set_access()
  call this%return()
end function

function Connectivity_constructor(name) result(this)
  use atlas_connectivity_c_binding
  use fckit_c_interop_module, only : c_str
  type(atlas_Connectivity) :: this
  character(len=*), intent(in), optional :: name
  call this%reset_c_ptr( atlas__Connectivity__create() )
  call this%set_access()
  if( present(name) ) then
    call atlas__Connectivity__rename(this%c_ptr(),c_str(name))
  endif
  call this%return()
end function


function atlas_Connectivity__name(this) result(name)
  use, intrinsic :: iso_c_binding, only : c_ptr
  use atlas_connectivity_c_binding
  use fckit_c_interop_module, only : c_ptr_to_string
  class(atlas_Connectivity), intent(in) :: this
  character(len=:), allocatable :: name
  type(c_ptr) :: name_c_str
  name_c_str = atlas__Connectivity__name(this%c_ptr())
  name = c_ptr_to_string(name_c_str)
end function

pure function access_value(this,c,r) result(val)
  use, intrinsic :: iso_c_binding, only : c_int, c_size_t
  integer(c_int) :: val
  class(atlas_ConnectivityAccess), intent(in) :: this
  integer(c_size_t), intent(in) :: r,c
  val = this%values_(c+this%displs_(r))
end function

pure function access_rows(this) result(val)
  use, intrinsic :: iso_c_binding, only : c_int
  integer(c_int) :: val
  class(atlas_ConnectivityAccess), intent(in) :: this
  val = this%rows_
end function

pure function atlas_Connectivity__value_args_size_t(this,c,r) result(val)
  use, intrinsic :: iso_c_binding, only : c_size_t, c_int
  integer(c_int) :: val
  class(atlas_Connectivity), intent(in) :: this
  integer(c_size_t), intent(in) :: r,c
  val = this%access%values_(c+this%access%displs_(r))
end function

pure function atlas_Connectivity__value_args_int(this,c,r) result(val)
  use, intrinsic :: iso_c_binding, only : c_int
  integer(c_int) :: val
  class(atlas_Connectivity), intent(in) :: this
  integer(c_int), intent(in) :: r,c
  val = this%access%values_(c+this%access%displs_(r))
end function

pure function atlas_Connectivity__rows(this) result(val)
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_Connectivity), intent(in) :: this
  val = this%access%rows_
end function

function atlas_Connectivity__missing_value(this) result(val)
  use atlas_connectivity_c_binding
  integer(c_int) :: val
  class(atlas_Connectivity), intent(in) :: this
  val = atlas__Connectivity__missing_value(this%c_ptr())
end function

pure function atlas_Connectivity__cols(this,r) result(val)
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_Connectivity), intent(in) :: this
  integer(c_size_t), intent(in) :: r
 val = this%access%cols(r)
end function

pure function atlas_Connectivity__mincols(this) result(val)
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_Connectivity), intent(in) :: this
  val = this%access%mincols_
end function

pure function atlas_Connectivity__maxcols(this) result(val)
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_Connectivity), intent(in) :: this
  val = this%access%maxcols_
end function

subroutine atlas_Connectivity__padded_data(this, padded, cols)
  use, intrinsic :: iso_c_binding, only : c_int, c_size_t
  class(atlas_Connectivity), intent(inout) :: this
  integer(c_int), pointer, intent(inout) :: padded(:,:)
  integer(c_size_t), pointer, intent(inout), optional :: cols(:)
  if( .not. associated(this%access%padded_) ) call update_padded(this%access)
  padded => this%access%padded_
  if( present(cols) ) cols => this%access%cols
end subroutine

function c_loc_int32(x)
  use, intrinsic :: iso_c_binding
  integer(c_int), target :: x
  type(c_ptr) :: c_loc_int32
  c_loc_int32 = c_loc(x)
end function

subroutine atlas_Connectivity__data(this, data, ncols)
  use, intrinsic :: iso_c_binding, only : c_int, c_f_pointer, c_loc
  class(atlas_Connectivity), intent(in) :: this
  integer(c_int), pointer, intent(inout) :: data(:,:)
  integer(c_int), intent(out), optional :: ncols
  integer(c_int) :: maxcols

  maxcols = this%maxcols()
  if( maxcols == this%mincols() ) then
    if( size(this%access%values_) > 0 ) then
      call c_f_pointer (c_loc_int32(this%access%values_(1)), data, &
          & (/maxcols,int(this%access%rows_,c_int)/))
      if( present(ncols) ) then
        ncols = maxcols
      endif
    endif
  else
    write(0,*) "ERROR: Cannot point connectivity pointer data(:,:) to irregular table"
  endif
end subroutine


subroutine atlas_Connectivity__row(this, row_idx, row, cols)
  use, intrinsic :: iso_c_binding, only : c_int
  class(atlas_Connectivity), intent(in) :: this
  integer(c_int), intent(in) :: row_idx
  integer(c_int), pointer, intent(inout) :: row(:)
  integer(c_int), intent(out) ::  cols
  row  => this%access%values_(this%access%displs_(row_idx)+1:this%access%displs_(row_idx+1)+1)
  cols =  this%access%cols(row_idx)
end subroutine

subroutine atlas_Connectivity__add_values_args_size_t(this,rows,cols,values)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t, c_int
  class(atlas_Connectivity), intent(in) :: this
  integer(c_size_t) :: rows
  integer(c_size_t) :: cols
  integer(c_int) :: values(:)
  call atlas__connectivity__add_values(this%c_ptr(),rows,cols,values)
end subroutine

subroutine atlas_Connectivity__add_values_args_int(this,rows,cols,values)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_int
  class(atlas_Connectivity), intent(in) :: this
  integer(c_int) :: rows
  integer(c_int) :: cols
  integer(c_int) :: values(:)
  call atlas__connectivity__add_values(this%c_ptr(),int(rows,c_size_t),int(cols,c_size_t),values)
end subroutine

subroutine atlas_Connectivity__add_missing_args_size_t(this,rows,cols)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t
  class(atlas_Connectivity), intent(in) :: this
  integer(c_size_t) :: rows
  integer(c_size_t) :: cols
  call atlas__connectivity__add_missing(this%c_ptr(),rows,cols)
end subroutine

subroutine atlas_Connectivity__add_missing_args_int(this,rows,cols)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_int
  class(atlas_Connectivity), intent(in) :: this
  integer(c_int) :: rows
  integer(c_int) :: cols
  call atlas__connectivity__add_missing(this%c_ptr(),int(rows,c_size_t),int(cols,c_size_t))
end subroutine

!========================================================

function MultiBlockConnectivity_cptr(cptr) result(this)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_ptr
  type(atlas_MultiBlockConnectivity) :: this
  type(c_ptr) :: cptr
  call this%reset_c_ptr( cptr )
  call this%set_access()
  call this%return()
end function

function MultiBlockConnectivity_constructor(name) result(this)
  use atlas_connectivity_c_binding
  use fckit_c_interop_module
  type(atlas_MultiBlockConnectivity) :: this
  character(len=*), intent(in), optional :: name
  call this%reset_c_ptr( atlas__MultiBlockConnectivity__create() )
  call this%set_access()
  if( present(name) ) then
    call atlas__Connectivity__rename(this%c_ptr(),c_str(name))
  endif
  call this%return()
end function

function atlas_MultiBlockConnectivity__blocks(this) result(val)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_MultiBlockConnectivity), intent(in) :: this
  val = atlas__MultiBlockConnectivity__blocks(this%c_ptr())
end function

function atlas_MultiBlockConnectivity__block(this,block_idx) result(block)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t
  type(atlas_BlockConnectivity) :: block
  class(atlas_MultiBlockConnectivity), intent(in) :: this
  integer(c_size_t) :: block_idx
  call block%reset_c_ptr( atlas__MultiBlockConnectivity__block(this%c_ptr(),block_idx-1) )
end function

!========================================================

function BlockConnectivity_cptr(cptr) result(this)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_ptr
  type(atlas_BlockConnectivity) :: this
  type(c_ptr) :: cptr
  call this%reset_c_ptr( cptr )
end function

subroutine atlas_BlockConnectivity__data(this,data)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_int, c_ptr, c_size_t, c_f_pointer
  class(atlas_BlockConnectivity), intent(in) :: this
  integer(c_int), pointer, intent(inout) :: data(:,:)
  type(c_ptr) :: data_cptr
  integer(c_size_t) :: rows
  integer(c_size_t) :: cols
  call atlas__BlockConnectivity__data(this%c_ptr(),data_cptr,rows,cols)
  call c_f_pointer (data_cptr, data, [cols,rows])
end subroutine

function atlas_BlockConnectivity__rows(this) result(val)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_BlockConnectivity), intent(in) :: this
  val = atlas__BlockConnectivity__rows(this%c_ptr())
end function

function atlas_BlockConnectivity__cols(this) result(val)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_size_t
  integer(c_size_t) :: val
  class(atlas_BlockConnectivity), intent(in) :: this
  val = atlas__BlockConnectivity__cols(this%c_ptr())
end function

function atlas_BlockConnectivity__missing_value(this) result(val)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_int
  integer(c_int) :: val
  class(atlas_BlockConnectivity), intent(in) :: this
  val = atlas__BlockConnectivity__missing_value(this%c_ptr()) + 1
end function

!========================================================

subroutine set_access(this)
  use, intrinsic :: iso_c_binding, only : c_ptr, c_int, c_f_pointer, c_funloc, c_loc
  class(atlas_Connectivity), intent(inout) :: this
  type(c_ptr) :: ctxt
  integer(c_int) :: have_ctxt
  have_ctxt = atlas__connectivity__ctxt(this%c_ptr(),ctxt)
  if( have_ctxt == 1 ) then
    call c_f_pointer(ctxt,this%access)
  else
    allocate( this%access )
    call atlas__connectivity__register_ctxt  ( this%c_ptr(), c_loc(this%access) )
    call atlas__connectivity__register_update( this%c_ptr(), c_funloc(update_access_c) )
    call atlas__connectivity__register_delete( this%c_ptr(), c_funloc(delete_access_c) )
    
    this%access%connectivity_ptr = this%c_ptr()
    call update_access(this%access)
  endif
end subroutine

subroutine update_access_c(this_ptr) bind(c)
  use, intrinsic :: iso_c_binding, only : c_ptr, c_f_pointer
  type(c_ptr), value :: this_ptr
  type(atlas_ConnectivityAccess), pointer :: this
  call c_f_pointer(this_ptr,this)
  call update_access(this)
end subroutine

subroutine update_access(this)
  use atlas_connectivity_c_binding
  use, intrinsic :: iso_c_binding, only : c_ptr, c_size_t, c_f_pointer
  type(atlas_ConnectivityAccess), intent(inout) :: this
  integer :: jrow

  type(c_ptr) :: values_cptr
  type(c_ptr) :: displs_cptr
  type(c_ptr) :: counts_cptr
  integer(c_size_t) :: values_size
  integer(c_size_t) :: displs_size
  integer(c_size_t) :: counts_size

  this%missing_value_ = atlas__Connectivity__missing_value(this%connectivity_ptr)
  call atlas__Connectivity__values(this%connectivity_ptr,values_cptr,values_size)
  call atlas__Connectivity__displs(this%connectivity_ptr,displs_cptr,displs_size)
  call atlas__Connectivity__counts(this%connectivity_ptr,counts_cptr,counts_size)

  call c_f_pointer(values_cptr, this%values_, (/values_size/))
  call c_f_pointer(displs_cptr, this%displs_, (/displs_size/))
  call c_f_pointer(counts_cptr, this%cols,    (/counts_size/))
  this%rows_   = atlas__Connectivity__rows(this%connectivity_ptr)
  if( associated( this%row ) ) deallocate(this%row)
  allocate( this%row(this%rows_) )
  this%maxcols_ = 0
  this%mincols_ = huge(this%maxcols_)
  do jrow=1,this%rows_
    this%row(jrow)%col => this%values_(this%displs_(jrow)+1:this%displs_(jrow+1)+1)
    this%maxcols_ = max(this%maxcols_, this%cols(jrow) )
    this%mincols_ = min(this%mincols_, this%cols(jrow) )
  enddo
  if( associated( this%padded_) ) call update_padded(this)
end subroutine

subroutine update_padded(this)
  use, intrinsic :: iso_c_binding, only : c_size_t
  class(atlas_ConnectivityAccess), intent(inout) :: this
  integer(c_size_t) :: jrow, jcol
  if( associated(this%padded_) ) deallocate(this%padded_)
  allocate(this%padded_(this%maxcols_,this%rows()))
  this%padded_(:,:) = this%missing_value_
  do jrow=1,this%rows()
    do jcol=1,this%cols(jrow)
      this%padded_(jcol,jrow) = this%value(jcol,jrow)
    enddo
  enddo
end subroutine

subroutine delete_access_c(this_ptr) bind(c)
  use, intrinsic :: iso_c_binding, only : c_ptr, c_f_pointer
  type(c_ptr), value :: this_ptr
  type(atlas_ConnectivityAccess), pointer :: this
  call c_f_pointer(this_ptr,this)
  call delete_access(this)
end subroutine

subroutine delete_access(this)
  use, intrinsic :: iso_c_binding, only : c_int
  use atlas_connectivity_c_binding
  type(atlas_ConnectivityAccess), intent(inout) :: this
  if( associated( this%row ) )    deallocate(this%row)
  if( associated( this%padded_) ) deallocate(this%padded_)
end subroutine

!-------------------------------------------------------------------------------

subroutine atlas_Connectivity__final_auto(this)
  type(atlas_Connectivity) :: this
#if FCKIT_FINAL_DEBUGGING
  write(0,*) "atlas_Connectivity__final_auto"
#endif
#if FCKIT_FINAL_NOT_PROPAGATING
  call this%final()
#endif
  FCKIT_SUPPRESS_UNUSED( this )
end subroutine

!-------------------------------------------------------------------------------

subroutine atlas_MultiBlockConnectivity__final_auto(this)
  type(atlas_MultiBlockConnectivity) :: this
#if FCKIT_FINAL_DEBUGGING
  write(0,*) "atlas_MultiBlockConnectivity__final_auto"
#endif
#if FCKIT_FINAL_NOT_PROPAGATING
  call this%final()
#endif
  FCKIT_SUPPRESS_UNUSED( this )
end subroutine

!-------------------------------------------------------------------------------

end module atlas_connectivity_module

